#pragma once

#include "renderer/ObserverRenderer.h"
#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"

class TestRenderer : public ObserverRenderer {

	~TestRenderer() override;

	void Init(HWND assochWnd, HINSTANCE instance) override;

	void Update() override;
	void Render() override;

	bool SupportsEditor() override { return true; }

private:
	std::unique_ptr<D3D11Graphics> m_gfx;
};
