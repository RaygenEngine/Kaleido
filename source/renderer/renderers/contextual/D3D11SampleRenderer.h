#pragma once

#include "renderer/ObserverRenderer.h"
#include "renderer/renderers/contextual/d3d11/renderable/D3D11GeometryGroup.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11ConstantBuffer.h"
#include "renderer/renderers/contextual/d3d11/asset/D3D11AssetManager.h"

template<typename T>
class D3D11VertexConstantBuffer;

class D3D11Graphics;
struct D3D11Model;

struct SceneModel {
	glm::mat4 transform;
	D3D11Model* model;
};

namespace d3d11 {

class D3D11SampleRenderer : public Renderer {

	~D3D11SampleRenderer() override;

	void Init(HWND assochWnd, HINSTANCE instance) override;

	void DrawFrame() override;

	bool SupportsEditor() override { return true; }

private:
	std::unique_ptr<D3D11Graphics> m_gfx;
	std::unique_ptr<D3D11AssetManager> m_assetManager;

	std::vector<SceneModel> m_sceneModels;

	// WIP: currently using only one transform for everything
	std::unique_ptr<D3D11VertexConstantBuffer<glm::mat4>> m_transform;
};

} // namespace d3d11
