#pragma once

namespace vk {
class CommandBuffer;
} // namespace vk

class D3D11Graphics;

// Provides an abstraction over the specific Win32 + ogl imgui implementation
class ImguiImpl {
public:
	static void InitContext();
	static void CleanupContext();

	static void NewFrame();
	static void EndFrame();

	static void InitOpenGL();
	static void RenderOpenGL();
	static void CleanupOpenGL();


	static void InitVulkan();
	static void RenderVulkan(vk::CommandBuffer* drawCommandBuffer);
	static void CleanupVulkan();

	static void InitD3D11(D3D11Graphics& gfx);
	static void CleanupD3D11();
	static void RenderD3D11();

	static void UpdateWindow(HWND hWnd);

	static LRESULT WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

	static inline HWND s_windowHandle;
};
