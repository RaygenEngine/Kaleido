#pragma once

#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"

#include <d3d11.h>

class D3D11Graphics;

class D3D11InputLayout : public D3D11Bindable {

public:
	D3D11InputLayout(
		D3D11Graphics& gfx, const std::vector<D3D11_INPUT_ELEMENT_DESC>& layout, ID3DBlob* pVertexShaderBytecode);
	void Bind() override;

private:
	WRL::ComPtr<ID3D11InputLayout> m_d3d11InputLayout;
};
