#pragma once

#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"

#include <d3d11.h>

class D3D11Graphics;

class D3D11VertexShader : public D3D11Bindable {
public:
	D3D11VertexShader(D3D11Graphics& gfx, const std::wstring& path);
	void Bind() override;
	ID3DBlob* GetBytecode() const;

protected:
	WRL::ComPtr<ID3DBlob> m_d3dBlob;
	WRL::ComPtr<ID3D11VertexShader> m_d3d11VertexShader;
};
