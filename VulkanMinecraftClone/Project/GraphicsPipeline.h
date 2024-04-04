#pragma once
#include <vulkan/vulkan.h>
#include "MachineShader.h"

class GraphicsPipeline 
{
public:
    GraphicsPipeline(const std::string& vertexShaderFile,
        const std::string& fragmentShaderFile)
        :
        m_Shaders{ vertexShaderFile, fragmentShaderFile },
        m_PipelineLayout{ VK_NULL_HANDLE },
        m_GraphicsPipeline{ VK_NULL_HANDLE }
    {}
    virtual ~GraphicsPipeline() {}

    // Methods for pipeline creation, configuration, and destruction
    virtual void CreatePipeline(VkDevice device, VkRenderPass renderPass) = 0;
    virtual void ConfigurePipeline() = 0;
    virtual void DestroyPipeline(VkDevice device) = 0;

    // Methods for rendering
    virtual void BindPipeline(VkCommandBuffer commandBuffer) = 0;

    VkPipelineLayout GetPipelineLayout() { return m_PipelineLayout; }
protected:
    VkPipelineLayout m_PipelineLayout;
    VkPipeline m_GraphicsPipeline;
    Shader m_Shaders;
};