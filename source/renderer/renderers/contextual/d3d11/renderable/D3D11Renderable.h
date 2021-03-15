#pragma once

#include "system/Logger.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"

#include <d3d11.h>

class D3D11Graphics;
class D3D11IndexBuffer;

class D3D11RenderableBase {

public:
	D3D11RenderableBase(D3D11Graphics& gfx);
	D3D11RenderableBase(const D3D11RenderableBase&) = delete;
	void Draw() const;
	virtual ~D3D11RenderableBase() = default;

protected:
	void AddBind(std::unique_ptr<D3D11Bindable> bind);
	void AddIndexBuffer(std::unique_ptr<D3D11IndexBuffer> ibuf);

	// private:
	virtual const std::vector<std::unique_ptr<D3D11Bindable>>& GetStaticBinds() const = 0;

	D3D11Graphics& m_gfx;

private:
	const D3D11IndexBuffer* m_indexBuffer = nullptr;
	std::vector<std::unique_ptr<D3D11Bindable>> binds;
};

template<class T>
class D3D11Renderable : public D3D11RenderableBase {
protected:
	D3D11Renderable(D3D11Graphics& gfx)
		: D3D11RenderableBase(gfx){};

	static bool IsStaticInitialized() noexcept { return !staticBinds.empty(); }

	static void AddStaticBind(std::unique_ptr<D3D11Bindable> bind)
	{
		CLOG_ABORT(typeid(*bind) == typeid(D3D11IndexBuffer), "Use AddIndexBuffer to bind index buffer");
		staticBinds.push_back(std::move(bind));
	}

	void AddStaticIndexBuffer(std::unique_ptr<D3D11IndexBuffer> ibuf)
	{
		CLOG_ABORT(m_indexBuffer != nullptr, "Can't add a bind to index buffer more than once");
		m_indexBuffer = ibuf.get();
		staticBinds.push_back(std::move(ibuf));
	}

	void SetIndexFromStatic()
	{
		CLOG_ABORT(m_indexBuffer != nullptr, "Can't add a bind to index buffer more than once");
		for (const auto& b : staticBinds) {
			if (const auto p = dynamic_cast<D3D11IndexBuffer*>(b.get())) {
				m_indexBuffer = p;
				return;
			}
		}
		LOG_ABORT(m_indexBuffer == nullptr, "Failed to find index buffer in static binds");
	}

private:
	const std::vector<std::unique_ptr<D3D11Bindable>>& GetStaticBinds() const noexcept override { return staticBinds; }

private:
	static std::vector<std::unique_ptr<D3D11Bindable>> staticBinds;
};

template<class T>
std::vector<std::unique_ptr<D3D11Bindable>> D3D11Renderable<T>::staticBinds;
