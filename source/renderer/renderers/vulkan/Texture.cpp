#include "pch/pch.h"

#include "renderer/renderers/vulkan/Model.h"
#include "asset/AssetManager.h"


namespace vlkn {
Texture::Texture(Device* device, PodHandle<TexturePod> handle)
{

	auto data = handle.Lock();

	// WIP: get first image for now
	auto imgData = data->images[0].Lock();

	// if(isHdr) data -> float* else data -> byte*
	vk::DeviceSize imageSize = imgData->height * imgData->width * imgData->components * (imgData->isHdr * 4);


	vk::UniqueBuffer stagingBuffer;
	vk::UniqueDeviceMemory stagingBufferMemory;
	device->CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
		stagingBufferMemory);

	// copy data to buffer
	void* bufferData = device->mapMemory(stagingBufferMemory.get(), 0, imageSize);
	memcpy(bufferData, imgData->data, static_cast<size_t>(imageSize));
	device->unmapMemory(stagingBufferMemory.get());

	// WIP: based on texture
	device->CreateImage(imgData->width, imgData->height, vk::Format::eR8G8B8A8Srgb, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, m_handle, m_memory);

	// transition from undefined to transfer layout
	device->TransitionImageLayout(
		m_handle.get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// copy
	device->CopyBufferToImage(
		stagingBuffer.get(), m_handle.get(), static_cast<uint32>(imgData->width), static_cast<uint32>(imgData->height));

	// finally transiton to graphics layout for shader access
	device->TransitionImageLayout(m_handle.get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eTransferDstOptimal,
		vk::ImageLayout::eShaderReadOnlyOptimal);
}
} // namespace vlkn
