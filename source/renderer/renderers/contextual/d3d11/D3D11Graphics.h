#pragma once

#include <d3d11.h>

class D3D11Graphics {

	friend class ImguiImpl;
	friend class D3D11Bindable;

public:
	D3D11Graphics(HWND hWnd);
	D3D11Graphics(const D3D11Graphics&) = delete;
	D3D11Graphics& operator=(const D3D11Graphics&) = delete;
	~D3D11Graphics() = default;
	void EndFrame();
	void DrawIndexed(UINT count);
	void ClearBuffer(const glm::vec4& color);


private:
	WRL::ComPtr<ID3D11Device> m_device;
	WRL::ComPtr<IDXGISwapChain> m_swap;
	WRL::ComPtr<ID3D11DeviceContext> m_context;
	WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
	WRL::ComPtr<ID3D11DepthStencilView> m_dsv; // WIP: remove this
};
