#pragma once

#include "vulkan/vulkan.hpp"


namespace vlkn {

class Device;
class Swapchain;
class GraphicsPipeline;
class Texture;

struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};

// WIP: rename (uniform descriptors??)
class Descriptors {

	// uniforms
	std::vector<vk::UniqueBuffer> m_uniformBuffers;
	std::vector<vk::UniqueDeviceMemory> m_uniformBuffersMemory;

	vk::UniqueDescriptorPool m_descriptorPool;
	std::vector<vk::UniqueDescriptorSet> m_descriptorSets;

	Device* m_assocDevice;

public:
	Descriptors(Device* device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline);

	std::vector<vk::Buffer> GetUniformBuffers() const { return vk::uniqueToRaw(m_uniformBuffers); }
	std::vector<vk::DeviceMemory> GetUniformBuffersMemory() const { return vk::uniqueToRaw(m_uniformBuffersMemory); }
	std::vector<vk::DescriptorSet> GetDescriptorSets() const { return vk::uniqueToRaw(m_descriptorSets); }

	vk::DescriptorPool GetDescriptorPool() { return m_descriptorPool.get(); }

	void UpdateSamplerImageDescriptorSet(Texture* texture);
};
} // namespace vlkn
