#include <Ryme/Pipeline.hpp>
#include <Ryme/Vertex.hpp>
#include <Ryme/Graphics.hpp>
#include <Ryme/Log.hpp>

namespace ryme {

Pipeline::Pipeline(Shader * shader)
    : _shader(shader)
{
    _inputAssemblyStateCreateInfo = vk::PipelineInputAssemblyStateCreateInfo()
        .setTopology(vk::PrimitiveTopology::eTriangleList)
        .setPrimitiveRestartEnable(false);

    _rasterizationStateCreateInfo = vk::PipelineRasterizationStateCreateInfo()
        .setDepthClampEnable(false)
        .setRasterizerDiscardEnable(false)
        .setPolygonMode(vk::PolygonMode::eFill)
        .setCullMode(vk::CullModeFlagBits::eNone)
        .setFrontFace(vk::FrontFace::eCounterClockwise)
        .setDepthBiasEnable(true)
        .setDepthBiasConstantFactor(0.0f)
        .setDepthBiasClamp(0.0f)
        .setDepthBiasSlopeFactor(0.0f)
        .setLineWidth(1.0f);

    _colorBlendAttachmentState = vk::PipelineColorBlendAttachmentState()
        .setBlendEnable(true)
        .setSrcColorBlendFactor(vk::BlendFactor::eOne)
        .setDstColorBlendFactor(vk::BlendFactor::eOne)
        .setColorBlendOp(vk::BlendOp::eAdd)
        .setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
        .setDstAlphaBlendFactor(vk::BlendFactor::eOne)
        .setAlphaBlendOp(vk::BlendOp::eAdd)
        .setColorWriteMask(
            vk::ColorComponentFlagBits::eR |
            vk::ColorComponentFlagBits::eG |
            vk::ColorComponentFlagBits::eB |
            vk::ColorComponentFlagBits::eA
        );

    _colorBlendStateCreateInfo = vk::PipelineColorBlendStateCreateInfo()
        .setLogicOpEnable(false)
        .setLogicOp(vk::LogicOp::eCopy)
        .setAttachmentCount(1)
        .setPAttachments(&_colorBlendAttachmentState)
        .setBlendConstants({ 0.0f, 0.0f, 0.0f, 0.0f });

    _multisampleStateCreateInfo = vk::PipelineMultisampleStateCreateInfo()
        .setRasterizationSamples(vk::SampleCountFlagBits::e1)
        .setSampleShadingEnable(false);

    _depthStencilStateCreateInfo = vk::PipelineDepthStencilStateCreateInfo()
        .setDepthTestEnable(true)
        .setDepthWriteEnable(true)
        .setDepthCompareOp(vk::CompareOp::eLess)
        .setDepthBoundsTestEnable(false)
        .setStencilTestEnable(false);
}

Pipeline::~Pipeline()
{
    Free();
}

void Pipeline::Create()
{
    assert(_shader); // TODO: Improve

    auto stageList = _shader->GetShaderStageList();
    auto pipelineLayout = _shader->GetPipelineLayout();

    auto bindingList = GetVertexInputBindingDescriptionList();
    auto attributeList = GetVertexInputAttributeDescriptionList();

    auto viewportStateCreateInfo = vk::PipelineViewportStateCreateInfo()
        .setViewportCount(1)
        .setScissorCount(1);

    List<vk::DynamicState> dynamicStateList = {
        vk::DynamicState::eViewport,
        vk::DynamicState::eScissor,
    };

    // TODO: eDepthBias, eStencilCompareMask, eStencilReference, eStencilWriteMask

    auto dynamicStateCreateInfo = vk::PipelineDynamicStateCreateInfo()
        .setDynamicStates(dynamicStateList);

    auto vertexInputStateCreateInfo = vk::PipelineVertexInputStateCreateInfo()
        .setVertexBindingDescriptionCount(bindingList.size())
        .setPVertexBindingDescriptions(bindingList.data())
        .setVertexAttributeDescriptionCount(attributeList.size())
        .setPVertexAttributeDescriptions(attributeList.data());

    auto pipelineCreateInfo = vk::GraphicsPipelineCreateInfo()
        .setStages(stageList)
        .setPVertexInputState(&vertexInputStateCreateInfo)
        .setPInputAssemblyState(&_inputAssemblyStateCreateInfo)
        // .setPTessellationState()
        .setPViewportState(&viewportStateCreateInfo)
        .setPRasterizationState(&_rasterizationStateCreateInfo)
        .setPMultisampleState(&_multisampleStateCreateInfo)
        .setPDepthStencilState(&_depthStencilStateCreateInfo)
        .setPColorBlendState(&_colorBlendStateCreateInfo)
        .setPDynamicState(&dynamicStateCreateInfo)
        .setRenderPass(Graphics::RenderPass)
        .setLayout(pipelineLayout);
        
    Free();

    vk::Result result;
    std::tie(result, _pipeline) = Graphics::Device.createGraphicsPipeline(nullptr, pipelineCreateInfo);
    // Result could also be vk::Result::ePipelineCompileRequiredEXT
    if (result != vk::Result::eSuccess) {
        vk::throwResultException(result, "vk::Device::createGraphicsPipeline");
    }
}

void Pipeline::Free()
{
    Graphics::Device.destroyPipeline(_pipeline);
}

bool Pipeline::Reload()
{
    return true;
}

} // namespace ryme