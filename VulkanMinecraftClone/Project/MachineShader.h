#pragma once
#include "vulkan\vulkan_core.h"
#include <string>
#include <vector>
#include "vulkanbase/VulkanUtil.h"
#include "Mesh2D.h"
#include "BlockMesh.h"

class Shader final
{
public:
	Shader(
		const std::string& vertexShaderFile,
		const std::string& fragmentShaderFile
	) :	m_VertexShaderFile{vertexShaderFile},
		m_FragmentShaderFile{fragmentShaderFile}
	{

	}

	// Shader stages and shader modules
	void Initialize(const VkDevice& device)
	{
		m_ShaderStages.clear();
		m_ShaderStages.emplace_back(CreateVertexShaderInfo(device));
		m_ShaderStages.emplace_back(CreateFragmentShaderInfo(device));
	}
	inline std::vector<VkPipelineShaderStageCreateInfo>& GetShaderStages(){	return m_ShaderStages; }

	//"shaders/shader.frag.spv"
	VkPipelineShaderStageCreateInfo CreateFragmentShaderInfo(const VkDevice& device) {
		std::vector<char> fragShaderCode = readFile(m_FragmentShaderFile);
		VkShaderModule fragShaderModule = CreateShaderModule(device, fragShaderCode);

		VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
		fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		fragShaderStageInfo.module = fragShaderModule;
		fragShaderStageInfo.pName = m_FSEntryPoint.c_str();

		return fragShaderStageInfo;
	}

	//"shaders/shader.vert.spv"
	VkPipelineShaderStageCreateInfo CreateVertexShaderInfo(const VkDevice& device) {
		std::vector<char> vertShaderCode = readFile(m_VertexShaderFile);
		VkShaderModule vertShaderModule = CreateShaderModule(device, vertShaderCode);

		VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
		vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
		vertShaderStageInfo.module = vertShaderModule;
		vertShaderStageInfo.pName = m_VSEntryPoint.c_str();

		return vertShaderStageInfo;
	}

	std::unique_ptr<VkPipelineVertexInputStateCreateInfo> CreateVertexInputStateInfo2D()
	{
		auto vertexInputInfo = std::make_unique<VkPipelineVertexInputStateCreateInfo>();
		vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = Vertex2D::getBindingDescription();
		auto attributeDescriptions = Vertex2D::getAttributeDescriptions();

		vertexInputInfo->vertexBindingDescriptionCount = 1;
		vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(2);
		vertexInputInfo->pVertexBindingDescriptions = bindingDescription.release();
		vertexInputInfo->pVertexAttributeDescriptions = attributeDescriptions.release();
		vertexInputInfo->flags = 0;

		return vertexInputInfo;
	}

	std::unique_ptr<VkPipelineVertexInputStateCreateInfo> CreateVertexInputStateInfo()
	{
		auto vertexInputInfo = std::make_unique<VkPipelineVertexInputStateCreateInfo>();
		vertexInputInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		auto bindingDescription = Vertex::getBindingDescription();
		auto attributeDescriptions = Vertex::getAttributeDescriptions();

		vertexInputInfo->vertexBindingDescriptionCount = 1;
		vertexInputInfo->vertexAttributeDescriptionCount = static_cast<uint32_t>(3);
		vertexInputInfo->pVertexBindingDescriptions = bindingDescription.release();
		vertexInputInfo->pVertexAttributeDescriptions = attributeDescriptions.release();
		vertexInputInfo->flags = 0;

		return vertexInputInfo;
	}

	VkPipelineInputAssemblyStateCreateInfo CreateInputAssemblyStateInfo()
	{
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		return inputAssembly;
	}

	VkShaderModule CreateShaderModule(const VkDevice& device, const std::vector<char>& code) 
	{
		VkShaderModuleCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		createInfo.codeSize = code.size();
		createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

		VkShaderModule shaderModule;
		if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
			throw std::runtime_error("failed to create shader module!");
		}

		return shaderModule;
	}

	void DestroyShaderModules(const VkDevice& device)
	{
		for (auto& shaderStage : m_ShaderStages)
		{
			vkDestroyShaderModule(device, shaderStage.module, nullptr);
		}
	}

	~Shader() = default;
private:
	std::string m_VertexShaderFile;
	std::string m_FragmentShaderFile;
	std::string m_VSEntryPoint{"main"};
	std::string m_FSEntryPoint{"main"};

	std::vector<VkPipelineShaderStageCreateInfo> m_ShaderStages;
};