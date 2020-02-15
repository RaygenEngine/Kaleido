#include "pch/pch.h"
#include "Descriptors.h"


#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/GraphicsPipeline.h"
#include "renderer/renderers/vulkan/Texture.h"


namespace vlkn {
// WIP: descriptor set should describe
// 1. material layout (samplers etc)
// 2. other uniforms
Descriptors::Descriptors(Device* device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline)
	: m_assocDevice(device)
{
	uint32 setCount = swapchain->GetImageCount();

	// uniform buffer (memory)

	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(setCount);
	m_uniformBuffersMemory.resize(setCount);

	for (size_t i = 0; i < setCount; i++) {
		m_assocDevice->CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_uniformBuffers[i],
			m_uniformBuffersMemory[i]);
	}

	// descriptor pool

	std::array<vk::DescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(setCount);
	poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(setCount);

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(static_cast<uint32_t>(setCount) + 2); // WIP: cause of imgui?

	m_descriptorPool = m_assocDevice->createDescriptorPoolUnique(poolInfo);

	// descriptor sets

	std::vector<vk::DescriptorSetLayout> layouts(setCount, graphicsPipeline->GetDescriptorSetLayout());
	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo.setDescriptorPool(m_descriptorPool.get())
		.setDescriptorSetCount(static_cast<uint32>(setCount))
		.setPSetLayouts(layouts.data());

	m_descriptorSets.resize(setCount);
	m_descriptorSets = m_assocDevice->allocateDescriptorSetsUnique(allocInfo);

	for (uint32 i = 0; i < setCount; ++i) {
		vk::DescriptorBufferInfo bufferInfo{};
		bufferInfo.setBuffer(m_uniformBuffers[i].get()).setOffset(0).setRange(sizeof(UniformBufferObject));

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.setDstSet(m_descriptorSets[i].get())
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		m_assocDevice->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}
void Descriptors::UpdateSamplerImageDescriptorSet(vlkn::Texture* texture)
{
	// PERF: not the right way to do this
	auto arr = vk::uniqueToRaw(m_descriptorSets);
	for (auto& ds : arr) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
			.setImageView(texture->GetView())
			.setSampler(texture->GetSampler());

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.setDstSet(ds)
			.setDstBinding(1)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		m_assocDevice->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}
} // namespace vlkn
