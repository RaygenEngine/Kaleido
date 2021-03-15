#include "TestRenderer.h"

#include "system/Engine.h"
#include "editor/imgui/ImguiImpl.h"


TestRenderer::~TestRenderer()
{
	if (Engine::IsEditorEnabled()) {
		ImguiImpl::CleanupD3D11();
	}
}

void TestRenderer::Init(HWND assochWnd, HINSTANCE instance)
{
	m_gfx = std::make_unique<D3D11Graphics>(assochWnd);

	if (Engine::IsEditorEnabled()) {
		ImguiImpl::InitD3D11(*m_gfx);
	}
}

void TestRenderer::Update()
{
	ObserverRenderer::Update();
}

void TestRenderer::Render()
{
	m_gfx->ClearBuffer({ 1.f, 0.f, 0.f, 1.f });


	if (Engine::IsEditorActive()) {
		ImguiImpl::RenderD3D11();
	}

	m_gfx->EndFrame();
}
