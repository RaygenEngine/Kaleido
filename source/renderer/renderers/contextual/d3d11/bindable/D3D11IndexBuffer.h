#pragma once

#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"
#include "renderer/renderers/contextual/d3d11/Error.h"

#include <d3d11.h>

class D3D11IndexBuffer : public D3D11Bindable {

public:
	template<class T>
	D3D11IndexBuffer(D3D11Graphics& gfx, const std::vector<T>& indices)
		: D3D11Bindable(gfx)
		, m_count((UINT)indices.size())
		, m_format(sizeof(T) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT)
	{
		D3D11_BUFFER_DESC bd{};
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(sizeof(T) * indices.size());
		bd.StructureByteStride = sizeof(T);
		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = indices.data();
		ABORT_IF_FAILED(GetDevice()->CreateBuffer(&bd, &sd, &m_d3d11Buffer));
	}

	void Bind() override;

	UINT GetCount() const { return m_count; }

private:
	UINT m_count;
	DXGI_FORMAT m_format;
	WRL::ComPtr<ID3D11Buffer> m_d3d11Buffer;
};
