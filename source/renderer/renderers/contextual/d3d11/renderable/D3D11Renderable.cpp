#include "D3D11Renderable.h"

#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11IndexBuffer.h"
#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"


D3D11RenderableBase::D3D11RenderableBase(D3D11Graphics& gfx)
	: m_gfx(gfx)
{
}

void D3D11RenderableBase::Draw() const
{
	for (auto& b : binds) {
		b->Bind();
	}
	for (auto& b : GetStaticBinds()) {
		b->Bind();
	}
	m_gfx.DrawIndexed(m_indexBuffer->GetCount());
}

void D3D11RenderableBase::AddBind(std::unique_ptr<D3D11Bindable> bind)
{
	CLOG_ABORT(typeid(*bind) == typeid(D3D11IndexBuffer), "Use AddIndexBuffer to bind index buffer");
	binds.push_back(std::move(bind));
}

void D3D11RenderableBase::AddIndexBuffer(std::unique_ptr<D3D11IndexBuffer> ibuf)
{
	CLOG_ABORT(m_indexBuffer != nullptr, "Can't add a bind to index buffer more than once");
	m_indexBuffer = ibuf.get();
	binds.push_back(std::move(ibuf));
}
