#ifndef RYME_SHADER_HPP
#define RYME_SHADER_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Containers.hpp>
#include <Ryme/Path.hpp>
#include <Ryme/Asset.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Shader : public Asset
{
public:

    Shader() = default;

    Shader(const List<Path>& pathList, bool search = true);

    virtual ~Shader();

    bool LoadFromFiles(const List<Path>& pathList, bool search = true);

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

    inline List<vk::PipelineShaderStageCreateInfo>& GetShaderStageList() {
        return _shaderStageCreateInfoList;
    }
    
    inline vk::PipelineLayout& GetPipelineLayout() {
        return _pipelineLayout;
    }

private:

    bool LoadSPV(const Path& path, bool search);

    List<Path> _pathList;

    List<vk::ShaderModule> _shaderModuleList;

    List<String> _entryPointNameList;

    List<vk::PipelineShaderStageCreateInfo> _shaderStageCreateInfoList;

    List<List<vk::DescriptorSetLayoutBinding>> _descriptorSetLayoutBindingListList;

    List<vk::PushConstantRange> _pushConstantRangeList;

    List<vk::DescriptorSetLayout> _descriptorSetLayoutList;

    vk::PipelineLayout _pipelineLayout;

}; // class Shader

} // namespace ryme

#endif // RYME_SHADER_HPP