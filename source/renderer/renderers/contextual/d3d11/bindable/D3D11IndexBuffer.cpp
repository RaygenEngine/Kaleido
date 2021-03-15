#include "D3D11IndexBuffer.h"

void D3D11IndexBuffer::Bind()
{
	GetContext()->IASetIndexBuffer(m_d3d11Buffer.Get(), m_format, 0u);
}
