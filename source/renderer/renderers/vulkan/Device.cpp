#include "pch/pch.h"

#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/PhysicalDevice.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/GraphicsPipeline.h"
#include "asset/AssetManager.h"
#include "system/Logger.h"

#include <set>

namespace vlkn {

Device::Device(vk::Device handle, PhysicalDevice* physicalDevice)
	: vk::Device(handle)
	, m_assocPhysicalDevice(physicalDevice)
{
	// WIP: should be ready here, dont Get them again
	auto graphicsQueueFamily = m_assocPhysicalDevice->GetBestGraphicsFamily();
	auto transferQueueFamily = m_assocPhysicalDevice->GetBestTransferFamily();
	auto presentQueueFamily = m_assocPhysicalDevice->GetBestPresentFamily();

	m_graphicsQueue = getQueue(graphicsQueueFamily.familyIndex, 0);
	m_transferQueue = getQueue(transferQueueFamily.familyIndex, 0);
	m_presentQueue = getQueue(presentQueueFamily.familyIndex, 0);

	vk::CommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.setQueueFamilyIndex(graphicsQueueFamily.familyIndex);
	graphicsPoolInfo.setFlags(vk::CommandPoolCreateFlags(0));

	m_graphicsCommandPool = createCommandPoolUnique(graphicsPoolInfo);

	vk::CommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.setQueueFamilyIndex(transferQueueFamily.familyIndex);
	transferPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);


	m_transferCommandPool = createCommandPoolUnique(transferPoolInfo);


	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(m_transferCommandPool.get())
		.setCommandBufferCount(1u);

	m_transferCommandBuffer = std::move(allocateCommandBuffersUnique(allocInfo)[0]);


	allocInfo.setCommandPool(m_graphicsCommandPool.get());

	m_graphicsCommandBuffer = std::move(allocateCommandBuffersUnique(allocInfo)[0]);
}

Device::~Device()
{
	destroy();
}

vk::UniqueShaderModule Device::CreateShaderModule(const std::string& binPath)
{
	auto& data = AssetManager::GetOrCreateFromParentUri<BinaryPod>(binPath, "/").Lock()->data;

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(data.size()).setPCode(reinterpret_cast<const uint32*>(data.data()));

	return createShaderModuleUnique(createInfo);
}

std::unique_ptr<Texture> Device::CreateTexture(PodHandle<TexturePod> textPod)
{
	return std::make_unique<Texture>(this, textPod);
}

std::unique_ptr<Swapchain> Device::RequestDeviceSwapchainOnSurface(vk::SurfaceKHR surface)
{
	return std::make_unique<Swapchain>(this, surface);
}

std::unique_ptr<GraphicsPipeline> Device::RequestDeviceGraphicsPipeline(Swapchain* swapchain)
{
	return std::make_unique<GraphicsPipeline>(this, swapchain);
}

std::unique_ptr<Descriptors> Device::RequestDeviceDescriptors(Swapchain* swapchain, GraphicsPipeline* pipeline)
{
	return std::make_unique<Descriptors>(this, swapchain, pipeline);
}

void Device::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory)
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

	buffer = createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = getBufferMemoryRequirements(buffer.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(m_assocPhysicalDevice->FindMemoryType(memRequirements.memoryTypeBits, properties));

	// From https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	// for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	// maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	// NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create
	// a custom allocator that splits up a single allocation among many different objects by using the offset
	// parameters that we've seen in many functions.
	memory = allocateMemoryUnique(allocInfo);

	bindBufferMemory(buffer.get(), memory.get(), 0);
}

void Device::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	m_transferCommandBuffer->begin(beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.setSize(size);

	m_transferCommandBuffer->copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	m_transferCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&m_transferCommandBuffer.get());

	m_transferQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	m_transferQueue.waitIdle();
}

void Device::CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
	vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::UniqueImage& image,
	vk::UniqueDeviceMemory& memory)
{
	auto pd = GetPhysicalDevice();

	vk::ImageCreateInfo imageInfo{};
	imageInfo.setImageType(vk::ImageType::e2D)
		.setExtent({ width, height, 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(tiling)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(usage)
		.setSamples(vk::SampleCountFlagBits::e1)
		// WIP: test
		.setSharingMode(vk::SharingMode::eExclusive);

	image = createImageUnique(imageInfo);

	vk::MemoryRequirements memRequirements = getImageMemoryRequirements(image.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(pd->FindMemoryType(memRequirements.memoryTypeBits, properties));

	memory = allocateMemoryUnique(allocInfo);

	bindImageMemory(image.get(), memory.get(), 0);
}

void Device::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32 width, uint32 height)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	m_transferCommandBuffer->begin(beginInfo);

	vk::BufferImageCopy region{};
	region.setBufferOffset(0u)
		.setBufferRowLength(0u)
		.setBufferImageHeight(0u)
		.setImageOffset({ 0, 0, 0 })
		.setImageExtent({ width, height, 1u });
	region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	m_transferCommandBuffer->copyBufferToImage(
		buffer, image, vk::ImageLayout::eTransferDstOptimal, std::array{ region });

	m_transferCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&m_transferCommandBuffer.get());

	m_transferQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	m_transferQueue.waitIdle();
}

void Device::TransitionImageLayout(
	vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::ImageMemoryBarrier barrier{};
	barrier.setOldLayout(oldLayout).setNewLayout(newLayout).setImage(image);

	barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0u)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	if (newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

		// if has stencil component
		if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else {
		barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	}

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	// undefined -> transfer
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags{ 0u });
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;

		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		// WIP should not be accessed from pd
		auto transferQueueFamily = m_assocPhysicalDevice->GetBestTransferFamily();
		barrier.setDstQueueFamilyIndex(transferQueueFamily.familyIndex);
	}
	// transfer -> graphics
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
			 && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

		// WIP should not be accessed from pd
		auto transferQueueFamily = m_assocPhysicalDevice->GetBestTransferFamily();
		barrier.setSrcQueueFamilyIndex(transferQueueFamily.familyIndex);
		// WIP should not be accessed from pd
		auto graphicsQueueFamily = m_assocPhysicalDevice->GetBestGraphicsFamily();
		barrier.setDstQueueFamilyIndex(graphicsQueueFamily.familyIndex);
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags{ 0u });
		barrier.setDstAccessMask(
			vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else {
		LOG_ABORT("Unsupported layout transition!");
	}

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	m_graphicsCommandBuffer->begin(beginInfo);

	m_graphicsCommandBuffer->pipelineBarrier(
		sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

	m_graphicsCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&m_graphicsCommandBuffer.get());

	m_graphicsQueue.submit(1u, &submitInfo, {});
	m_graphicsQueue.waitIdle();
}

vk::UniqueImageView Device::CreateImageView(vk::Image image, vk::Format format)
{
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.setImage(image).setViewType(vk::ImageViewType::e2D).setFormat(format);
	viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0u)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	return createImageViewUnique(viewInfo);
}

vk::Result Device::Present(const vk::PresentInfoKHR& info)
{
	return m_presentQueue.presentKHR(info);
}

vk::Result Device::SubmitGraphics(uint32 submitCount, vk::SubmitInfo* pSubmits, vk::Fence fence)
{
	return m_graphicsQueue.submit(submitCount, pSubmits, fence);
}

} // namespace vlkn
