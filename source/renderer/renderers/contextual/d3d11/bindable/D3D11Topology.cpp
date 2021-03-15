#include "D3D11Topology.h"

D3D11Topology::D3D11Topology(D3D11Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type)
	: D3D11Bindable(gfx)
	, m_type(type)
{
}

void D3D11Topology::Bind()
{
	GetContext()->IASetPrimitiveTopology(m_type);
}
