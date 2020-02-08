#pragma once

#include "renderer/renderers/vulkan/Device.h"
#include "asset/pods/ModelPod.h"

#include "vulkan/vulkan.hpp"

namespace vlkn {

class Texture {

	vk::UniqueImage m_handle;
	vk::UniqueDeviceMemory m_memory;

public:
	Texture(Device* device, PodHandle<TexturePod> handle);
};
} // namespace vlkn
