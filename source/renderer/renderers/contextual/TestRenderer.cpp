#include "TestRenderer.h"

#include "system/Engine.h"
#include "editor/imgui/ImguiImpl.h"

#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/World.h"


#include "renderer/renderers/contextual/d3d11/bindable/D3D11ConstantBuffer.h"

#include <d3d11.h>

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


	// Fill the container with the current nodes of this type
	auto world = Engine::GetWorld();
	for (auto node : world->GetNodeIterator<GeometryNode>()) {
		auto model = node->GetModel().Lock();

		for (auto m : model->meshes) {
			for (auto gg : m.geometryGroups) {
				toRender.emplace_back(
					std::make_unique<D3D11GeometryGroup>(*m_gfx, gg, *model->materials.at(gg.materialIndex).Lock()));
			}
		}
	}

	transform = std::make_unique<D3D11VertexConstantBuffer<glm::mat4>>(*m_gfx);
}

void TestRenderer::Update()
{
	ObserverRenderer::Update();

	m_activeCamera = Engine::GetWorld()->GetActiveCamera();
	CLOG_WARN(!m_activeCamera, "Renderer failed to find camera.");

	auto vp = glm::transpose(m_activeCamera->GetViewProjectionMatrix());

	m_activeCamera;
	transform->Update(vp);
}

void TestRenderer::Render()
{
	m_gfx->ClearBuffer({ 0.f, 0.f, 0.f, 1.f });

	transform->Bind();
	for (auto& vgg : toRender) {
		vgg->Draw();
	}


	if (Engine::IsEditorActive()) {
		ImguiImpl::RenderD3D11();
	}

	m_gfx->EndFrame();
}
