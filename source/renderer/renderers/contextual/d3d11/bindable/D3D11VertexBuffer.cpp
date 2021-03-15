#include "D3D11VertexBuffer.h"

void D3D11VertexBuffer::Bind()
{
	const UINT offset = 0u;
	GetContext()->IASetVertexBuffers(0u, 1u, m_d3d11Buffer.GetAddressOf(), &m_stride, &offset);
}
