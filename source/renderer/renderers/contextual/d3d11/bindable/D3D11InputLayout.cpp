#include "D3D11InputLayout.h"

#include "renderer/renderers/contextual/d3d11/Error.h"

#include <d3d11.h>

D3D11InputLayout::D3D11InputLayout(
	D3D11Graphics& gfx, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout, ID3DBlob* pVertexShaderBytecode)
	: D3D11Bindable(gfx)
{
	ABORT_IF_FAILED(GetDevice()->CreateInputLayout(layout.data(), (UINT)layout.size(),
		pVertexShaderBytecode->GetBufferPointer(), pVertexShaderBytecode->GetBufferSize(), &m_d3d11InputLayout));
}

void D3D11InputLayout::Bind()
{
	GetContext()->IASetInputLayout(m_d3d11InputLayout.Get());
}
