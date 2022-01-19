#ifndef RYME_PIPELINE_HPP
#define RYME_PIPELINE_HPP

#include <Ryme/Config.hpp>
#include <Ryme/Asset.hpp>
#include <Ryme/Shader.hpp>

#include <Ryme/ThirdParty/vulkan.hpp>

namespace ryme {

class RYME_API Pipeline : public Asset
{
public:

    Pipeline(Shader * shader);

    virtual ~Pipeline();

    void Create();

    void Free() override;

    bool Reload() override;

    bool CanReload() const override {
        return true;
    }

    inline vk::Pipeline& GetPipeline() {
        return _pipeline;
    }
    
    // TODO: Add getters/setters

private:

    Shader * _shader = nullptr;

    vk::PipelineInputAssemblyStateCreateInfo _inputAssemblyStateCreateInfo;

    vk::PipelineRasterizationStateCreateInfo _rasterizationStateCreateInfo;

    vk::PipelineColorBlendAttachmentState _colorBlendAttachmentState;

    vk::PipelineColorBlendStateCreateInfo _colorBlendStateCreateInfo;

    vk::PipelineMultisampleStateCreateInfo _multisampleStateCreateInfo;

    vk::PipelineDepthStencilStateCreateInfo _depthStencilStateCreateInfo;

    vk::PipelineLayout _pipelineLayout;

    bool _needReload = false;

    vk::Pipeline _pipeline;

}; // class Pipeline

} // namespace ryme

#endif // RYME_PIPELINE_HPP