#pragma once

#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"
#include "renderer/renderers/contextual/d3d11/Error.h"

template<typename C>
class D3D11ConstantBuffer : public D3D11Bindable {
public:
	void Update(const C& consts)
	{
		D3D11_MAPPED_SUBRESOURCE msr;
		ABORT_IF_FAILED(GetContext()->Map(m_d3d11Buffer.Get(), 0u, D3D11_MAP_WRITE_DISCARD, 0u, &msr));
		memcpy(msr.pData, &consts, sizeof(consts));
		GetContext()->Unmap(m_d3d11Buffer.Get(), 0u);
	}

	D3D11ConstantBuffer(D3D11Graphics& gfx, const C& consts)
		: D3D11Bindable(gfx)
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(consts);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd{};
		csd.pSysMem = &consts;
		ABORT_IF_FAILED(GetDevice()->CreateBuffer(&cbd, &csd, &m_d3d11Buffer));
	}

	D3D11ConstantBuffer(D3D11Graphics& gfx)
		: D3D11Bindable(gfx)
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(C);
		cbd.StructureByteStride = 0u;
		ABORT_IF_FAILED(GetDevice()->CreateBuffer(&cbd, nullptr, &m_d3d11Buffer));
	}

protected:
	WRL::ComPtr<ID3D11Buffer> m_d3d11Buffer;
};

template<typename C>
class D3D11VertexConstantBuffer : public D3D11ConstantBuffer<C> {
	using D3D11ConstantBuffer<C>::m_d3d11Buffer;
	using D3D11Bindable::GetContext;

public:
	using D3D11ConstantBuffer<C>::D3D11ConstantBuffer;

	D3D11VertexConstantBuffer(D3D11Graphics& gfx)
		: D3D11ConstantBuffer<C>(gfx){};

	void Bind() override { GetContext()->VSSetConstantBuffers(0u, 1u, m_d3d11Buffer.GetAddressOf()); }
};

template<typename C>
class D3D11PixelConstantBuffer : public D3D11ConstantBuffer<C> {
	using D3D11ConstantBuffer<C>::m_d3d11Buffer;
	using D3D11Bindable::GetContext;

public:
	using D3D11ConstantBuffer<C>::D3D11ConstantBuffer;

	D3D11PixelConstantBuffer(D3D11Graphics& gfx)
		: D3D11ConstantBuffer<C>(gfx){};

	void Bind() override { GetContext()->PSSetConstantBuffers(0u, 1u, m_d3d11Buffer.GetAddressOf()); }
};
