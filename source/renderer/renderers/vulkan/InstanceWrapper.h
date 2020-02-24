#pragma once


#include "renderer/renderers/vulkan/PhysicalDeviceWrapper.h"

#include <windows.h>
#include <vulkan/vulkan.hpp>


namespace vlkn {

// Instance layer wrapper
class InstanceWrapper : public VkUniqueObjectWrapper<vk::UniqueInstance> {
	// TODO: currently can't add as unique
	vk::SurfaceKHR m_surface;

	// WIP: change mem management here
	std::vector<PhysicalDeviceWrapper> m_capablePhysicalDevices;

	vk::UniqueDebugUtilsMessengerEXT m_debugUtilsMessenger;

public:
	void Init(HWND assochWnd, HINSTANCE instance);
	~InstanceWrapper();

	vk::SurfaceKHR GetSurface() { return m_surface; }

	PhysicalDeviceWrapper& GetBestCapablePhysicalDevice();
};
} // namespace vlkn
