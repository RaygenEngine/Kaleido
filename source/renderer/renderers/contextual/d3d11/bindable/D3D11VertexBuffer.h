#pragma once

#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"
#include "renderer/renderers/contextual/d3d11/Error.h"

#include <d3d11.h>

class D3D11VertexBuffer : public D3D11Bindable {

public:
	template<class V>
	D3D11VertexBuffer(D3D11Graphics& gfx, const std::vector<V>& vertices)
		: D3D11Bindable(gfx)
		, m_stride(sizeof(V))
	{
		D3D11_BUFFER_DESC bd{};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(sizeof(V) * vertices.size());
		bd.StructureByteStride = sizeof(V);
		D3D11_SUBRESOURCE_DATA sd{};
		sd.pSysMem = vertices.data();
		ABORT_IF_FAILED(GetDevice()->CreateBuffer(&bd, &sd, &m_d3d11Buffer));
	}

	void Bind() override;

private:
	UINT m_stride;
	WRL::ComPtr<ID3D11Buffer> m_d3d11Buffer;
};
