#include "D3D11Graphics.h"

#include "renderer/renderers/contextual/d3d11/Error.h"

#include "glm/gtc/type_ptr.hpp"

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "D3DCompiler.lib")

D3D11Graphics::D3D11Graphics(HWND hWnd)
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 0;
	sd.BufferDesc.RefreshRate.Denominator = 0;
	sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = 1;
	sd.OutputWindow = hWnd;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	sd.Flags = 0;

	UINT swapCreateFlags = 0u;
#ifndef NDEBUG
	swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	// create device and front/back buffers, and swap chain and rendering context
	ABORT_IF_FAILED(D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, swapCreateFlags, nullptr,
		0, D3D11_SDK_VERSION, &sd, &m_swap, &m_device, nullptr, &m_context));

	// gain access to texture subresource in swap chain (back buffer)
	WRL::ComPtr<ID3D11Resource> pBackBuffer;
	ABORT_IF_FAILED(m_swap->GetBuffer(0, __uuidof(ID3D11Resource), &pBackBuffer));
	ABORT_IF_FAILED(m_device->CreateRenderTargetView(pBackBuffer.Get(), nullptr, &m_rtv));

	// create depth stensil state
	D3D11_DEPTH_STENCIL_DESC dsDesc = {};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	dsDesc.DepthFunc = D3D11_COMPARISON_LESS;
	WRL::ComPtr<ID3D11DepthStencilState> pDSState;
	ABORT_IF_FAILED(m_device->CreateDepthStencilState(&dsDesc, &pDSState));

	// bind depth state
	m_context->OMSetDepthStencilState(pDSState.Get(), 1u);

	// create depth stensil texture
	WRL::ComPtr<ID3D11Texture2D> pDepthStencil;
	D3D11_TEXTURE2D_DESC descDepth = {};
	descDepth.Width = 1904u;
	descDepth.Height = 1041u;
	descDepth.MipLevels = 1u;
	descDepth.ArraySize = 1u;
	descDepth.Format = DXGI_FORMAT_D32_FLOAT;
	descDepth.SampleDesc.Count = 1u;
	descDepth.SampleDesc.Quality = 0u;
	descDepth.Usage = D3D11_USAGE_DEFAULT;
	descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ABORT_IF_FAILED(m_device->CreateTexture2D(&descDepth, nullptr, &pDepthStencil));

	// create view of depth stensil texture
	D3D11_DEPTH_STENCIL_VIEW_DESC descDSV = {};
	descDSV.Format = DXGI_FORMAT_D32_FLOAT;
	descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	descDSV.Texture2D.MipSlice = 0u;
	ABORT_IF_FAILED(m_device->CreateDepthStencilView(pDepthStencil.Get(), &descDSV, &m_dsv));

	// bind depth stensil view to OM
	m_context->OMSetRenderTargets(1u, m_rtv.GetAddressOf(), m_dsv.Get());

	// configure viewport
	D3D11_VIEWPORT vp;
	vp.Width = 1904.f;
	vp.Height = 1041.0f;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0.0f;
	vp.TopLeftY = 0.0f;
	m_context->RSSetViewports(1u, &vp);
}

void D3D11Graphics::EndFrame()
{
	ABORT_IF_FAILED(m_swap->Present(1u, 0u));
}

void D3D11Graphics::DrawIndexed(UINT count)
{
	m_context->DrawIndexed(count, 0u, 0u);
}

void D3D11Graphics::ClearBuffer(const glm::vec4& color)
{
	m_context->ClearRenderTargetView(m_rtv.Get(), glm::value_ptr(color));
	m_context->ClearDepthStencilView(m_dsv.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
}
