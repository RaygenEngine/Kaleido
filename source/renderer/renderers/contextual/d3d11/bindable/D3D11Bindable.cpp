#include "D3D11Bindable.h"

#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"

D3D11Bindable::D3D11Bindable(D3D11Graphics& gfx)
	: m_gfx(gfx){};

ID3D11DeviceContext* D3D11Bindable::GetContext()
{
	return m_gfx.m_context.Get();
}

ID3D11Device* D3D11Bindable::GetDevice()
{
	return m_gfx.m_device.Get();
}
