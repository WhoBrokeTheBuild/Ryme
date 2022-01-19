#include <Ryme/Shader.hpp>
#include <Ryme/Exception.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>

#include <fstream>

#include <spirv_cross/spirv_cross.hpp>

namespace ryme {

Shader::Shader(const List<Path>& pathList, bool search /*= true*/)
{
    LoadFromFiles(pathList, search);
}

Shader::~Shader()
{
    Free();
}

bool Shader::LoadFromFiles(const List<Path>& pathList, bool search /*= true*/)
{
    Free();

    for (const auto& path : pathList) {
        if (!LoadSPV(path, search)) {
            if (!LoadSPV(path + ".spv", search)) {
                return false;
            }
        }
    }

    for (auto& bindingList : _descriptorSetLayoutBindingListList) {
        auto descriptorSetLayoutCreateInfo = vk::DescriptorSetLayoutCreateInfo()
            .setBindings(bindingList);

        auto descriptorSetLayout = Graphics::Device.createDescriptorSetLayout(descriptorSetLayoutCreateInfo);
        _descriptorSetLayoutList.push_back(descriptorSetLayout);
    }

    auto pipelineLayoutCreateInfo = vk::PipelineLayoutCreateInfo()
        .setSetLayouts(_descriptorSetLayoutList)
        .setPushConstantRanges(_pushConstantRangeList);

    _pipelineLayout = Graphics::Device.createPipelineLayout(pipelineLayoutCreateInfo);
    
    // If this is done in LoadSPV, the c_str gets moved when the vector resizes
    for (size_t i = 0; i < _shaderStageCreateInfoList.size(); ++i) {
        _shaderStageCreateInfoList[i].pName = _entryPointNameList[i].c_str();
    }

    _isLoaded = true;
    return true;
}

void Shader::Free()
{
    for (auto& shaderModule : _shaderModuleList) {
        Graphics::Device.destroyShaderModule(shaderModule);
    }
    _shaderModuleList.clear();
    
    for (auto& descriptorSetLayout : _descriptorSetLayoutList) {
        Graphics::Device.destroyDescriptorSetLayout(descriptorSetLayout);
    }
    _descriptorSetLayoutList.clear();

    Graphics::Device.destroyPipelineLayout(_pipelineLayout);

    _entryPointNameList.clear();

    _shaderStageCreateInfoList.clear();

    _descriptorSetLayoutBindingListList.clear();

    _pushConstantRangeList.clear();

    _isLoaded = false;
}

bool Shader::Reload()
{
    List<Path> oldPathList = _pathList;
    _pathList.clear();

    return LoadFromFiles(oldPathList, false);
}

bool Shader::LoadSPV(const Path& path, bool search)
{
    std::ifstream file;
    Path fullPath = path;

    if (search) {
        for (const auto& assetPath : GetAssetPathList()) {
            fullPath = assetPath / path;

            file.open(fullPath, std::ios::binary);
            if (file.is_open()) {
                break;
            }
        }
    }
    else {
        file.open(path, std::ios::binary);
    }

    if (!file.is_open()) {
        return false;
    }

    file.unsetf(std::ios::skipws);

    List<uint8_t> rawData(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    Span<uint32_t> data(
        reinterpret_cast<uint32_t *>(rawData.data()),
        rawData.size() / sizeof(uint32_t)
    );

    spirv_cross::Compiler compiler(data.data(), data.size());

    auto resources = compiler.get_shader_resources();
    auto entryPointStage = compiler.get_entry_points_and_stages().front();

    vk::ShaderStageFlagBits stage;
    switch (entryPointStage.execution_model) {
    case spv::ExecutionModel::ExecutionModelVertex:
        stage = vk::ShaderStageFlagBits::eVertex;
        break;
    case spv::ExecutionModel::ExecutionModelFragment:
        stage = vk::ShaderStageFlagBits::eFragment;
        break;
    case spv::ExecutionModel::ExecutionModelTessellationControl:
        stage = vk::ShaderStageFlagBits::eTessellationControl;
        break;
    case spv::ExecutionModel::ExecutionModelTessellationEvaluation:
        stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
        break;
    case spv::ExecutionModel::ExecutionModelGeometry:
        stage = vk::ShaderStageFlagBits::eGeometry;
        break;
    default:
        throw Exception("Invalid SPIR-V Execution Model: {}", entryPointStage.execution_model);
    }

    auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
        .setCodeSize(data.size_bytes())
        .setPCode(data.data());

    auto shaderModule = Graphics::Device.createShaderModule(shaderModuleCreateInfo);

    _shaderModuleList.push_back(shaderModule);

    _entryPointNameList.push_back(entryPointStage.name);

    auto shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo()
        .setStage(stage)
        .setModule(shaderModule)
        .setPName(_entryPointNameList.back().c_str())
        // .setPName("main")
        .setPSpecializationInfo(nullptr);

    _shaderStageCreateInfoList.push_back(shaderStageCreateInfo);

    // for (auto& resource : resources.stage_inputs) {
    //     auto location = compiler.get_decoration(resource.id, spv::Decoration::DecorationLocation);

    //     Log(RYME_ANCHOR, "{} location={}", resource.name, location);
    // }

    // for (auto& resource : resources.stage_outputs) {
    //     auto location = compiler.get_decoration(resource.id, spv::Decoration::DecorationLocation);

    //     Log(RYME_ANCHOR, "{} location={}", resource.name, location);
    // }

    for (auto& resource : resources.sampled_images) {
        uint32_t set = compiler.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(resource.id, spv::Decoration::DecorationBinding);
        
        if (set >= _descriptorSetLayoutBindingListList.size()) {
            _descriptorSetLayoutBindingListList.resize(set + 1);
        }

        auto& bindingList = _descriptorSetLayoutBindingListList[set];

        auto it = std::find_if(
            bindingList.begin(),
            bindingList.end(),
            [&](auto& setLayoutBinding) {
                return (setLayoutBinding.binding == binding);
            }
        );

        if (it == bindingList.end()) {
            bindingList.push_back(
                vk::DescriptorSetLayoutBinding()
                    .setBinding(binding)
                    .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
                    .setDescriptorCount(1)
                    .setStageFlags(stage)
            );
        }
        else {
            (*it).stageFlags |= stage;
        }
    }

    for (auto& resource : resources.uniform_buffers) {
        const auto& type = compiler.get_type(resource.base_type_id);
        size_t size = compiler.get_declared_struct_size(type);

        uint32_t set = compiler.get_decoration(resource.id, spv::Decoration::DecorationDescriptorSet);
        uint32_t binding = compiler.get_decoration(resource.id, spv::Decoration::DecorationBinding);
        
        if (set >= _descriptorSetLayoutBindingListList.size()) {
            _descriptorSetLayoutBindingListList.resize(set + 1);
        }

        auto& bindingList = _descriptorSetLayoutBindingListList[set];

        auto it = std::find_if(
            bindingList.begin(),
            bindingList.end(),
            [&](auto& setLayoutBinding) {
                return (setLayoutBinding.binding == binding);
            }
        );

        if (it == bindingList.end()) {
            bindingList.push_back(
                vk::DescriptorSetLayoutBinding()
                    .setBinding(binding)
                    .setDescriptorType(vk::DescriptorType::eUniformBuffer)
                    .setDescriptorCount(1)
                    .setStageFlags(stage)
            );
        }
        else {
            (*it).stageFlags |= stage;
        }
    }

    if (!resources.push_constant_buffers.empty()) {
        auto& resource = resources.push_constant_buffers.front();
        const auto& type = compiler.get_type(resource.base_type_id);
        size_t size = compiler.get_declared_struct_size(type);

        if (_pushConstantRangeList.empty()) {
            _pushConstantRangeList.push_back(
                vk::PushConstantRange()
                    .setOffset(0)
                    .setSize(size)
                    .setStageFlags(stage)
            );
        }
        else {
            _pushConstantRangeList.front().stageFlags |= stage;
        }
    }

    _pathList.push_back(fullPath);

    Log(RYME_ANCHOR, "Loaded '{}'", fullPath);

    return true;
}

} // namespace ryme