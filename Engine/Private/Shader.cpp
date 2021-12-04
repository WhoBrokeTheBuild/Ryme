#include <Ryme/Shader.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>

#include <fstream>

namespace ryme {

Shader::Shader(const List<Path>& pathList, bool search /*= true*/)
{
    LoadFromFiles(pathList, search);
}

bool Shader::LoadFromFiles(const List<Path>& pathList, bool search /*= true*/)
{
    for (const auto& path : pathList) {
        if (!LoadSPV(path, search)) {
            if (!LoadSPV(path + ".spv", search)) {
                return false;
            }
        }
    }

    _isLoaded = true;
    return true;
}

void Shader::Free()
{
    auto vkDevice = Graphics::GetVkDevice();

    for (auto& shaderModule : _shaderModuleList) {
        vkDevice.destroyShaderModule(shaderModule);
    }
    _shaderModuleList.clear();

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

    List<uint8_t> data(
        (std::istreambuf_iterator<char>(file)),
        std::istreambuf_iterator<char>()
    );

    String ext = fullPath.GetStem().GetExtension();

    vk::ShaderStageFlagBits stage;
    if (ext == "vert" || ext == "vertex") {
        stage = vk::ShaderStageFlagBits::eVertex;
    }
    else if (ext == "frag" || ext == "fragment") {
        stage = vk::ShaderStageFlagBits::eFragment;
    }
    else if (ext == "tesc" || ext == "tesscontrol") {
        stage = vk::ShaderStageFlagBits::eTessellationControl;
    }
    else if (ext == "tese" || ext == "tesseval") {
        stage = vk::ShaderStageFlagBits::eTessellationEvaluation;
    }
    else if (ext == "comp" || ext == "compute") {
        stage = vk::ShaderStageFlagBits::eCompute;
    }
    else {
        return false;
    }

    auto shaderModuleCreateInfo = vk::ShaderModuleCreateInfo()
        .setCodeSize(data.size())
        .setPCode(reinterpret_cast<uint32_t *>(data.data()));

    auto shaderModule = Graphics::GetVkDevice().createShaderModule(shaderModuleCreateInfo);

    _shaderModuleList.push_back(shaderModule);

    auto shaderStageCreateInfo = vk::PipelineShaderStageCreateInfo()
        .setStage(stage)
        .setModule(shaderModule)
        .setPName("main")
        .setPSpecializationInfo(nullptr);

    _shaderStageCreateInfoList.push_back(shaderStageCreateInfo);

    _pathList.push_back(fullPath);

    Log(RYME_ANCHOR, "Loaded '{}'", fullPath);

    return true;
}

} // namespace ryme