#pragma once

#include <d3d11.h>

class D3D11Graphics;

class D3D11Bindable {
public:
	// secures than we use the same device and context for binding, with the one we created the bindable
	D3D11Bindable(D3D11Graphics& gfx);
	virtual void Bind() = 0;
	virtual ~D3D11Bindable() = default;

protected:
	ID3D11DeviceContext* GetContext();
	ID3D11Device* GetDevice();

private:
	D3D11Graphics& m_gfx;
};
