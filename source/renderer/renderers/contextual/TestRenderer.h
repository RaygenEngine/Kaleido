#pragma once

#include "renderer/ObserverRenderer.h"
#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"
#include "renderer/renderers/contextual/d3d11/renderable/D3D11GeometryGroup.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11ConstantBuffer.h"

template<typename T>
class D3D11VertexConstantBuffer;

class D3D11GeometryGroup;

class TestRenderer : public ObserverRenderer {

	~TestRenderer() override;

	void Init(HWND assochWnd, HINSTANCE instance) override;

	void Update() override;
	void Render() override;

	bool SupportsEditor() override { return true; }

private:
	std::unique_ptr<D3D11Graphics> m_gfx;
	// WIP: asset model - do before textures
	std::vector<std::unique_ptr<D3D11GeometryGroup>> toRender;

	// WIP: currently using only one transform for everything
	std::unique_ptr<D3D11VertexConstantBuffer<glm::mat4>> transform;
};
