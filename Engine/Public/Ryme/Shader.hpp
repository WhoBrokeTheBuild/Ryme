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

    Shader(const List<Path>& pathList, bool search = true);

    bool LoadFromFiles(const List<Path>& pathList, bool search = true);

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

private:

    bool LoadSPV(const Path& path, bool search);

    List<Path> _pathList;

    List<vk::ShaderModule> _shaderModuleList;

    List<vk::PipelineShaderStageCreateInfo> _shaderStageCreateInfoList;

}; // class Shader

} // namespace ryme

#endif // RYME_SHADER_HPP