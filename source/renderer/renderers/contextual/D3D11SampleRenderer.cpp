#include "D3D11SampleRenderer.h"

#include "system/Engine.h"
#include "editor/imgui/ImguiImpl.h"

#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "world/World.h"


#include "renderer/renderers/contextual/d3d11/D3D11Graphics.h"
#include "renderer/renderers/contextual/d3d11/asset/D3D11Model.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11ConstantBuffer.h"

#include <d3d11.h>

using namespace d3d11;

D3D11SampleRenderer::~D3D11SampleRenderer()
{
	if (Engine::IsEditorEnabled()) {
		ImguiImpl::CleanupD3D11();
	}
}

void D3D11SampleRenderer::Init(HWND assochWnd, HINSTANCE instance)
{
	m_gfx = std::make_unique<D3D11Graphics>(assochWnd);
	m_assetManager = std::make_unique<D3D11AssetManager>(*m_gfx);

	if (Engine::IsEditorEnabled()) {
		ImguiImpl::InitD3D11(*m_gfx);
	}


	// Fill the container with the current nodes of this type
	auto world = Engine::GetWorld();
	for (auto node : world->GetNodeIterator<GeometryNode>()) {

		m_sceneModels.push_back(
			{ node->GetNodeTransformWCS(), m_assetManager->GpuGetOrCreate<D3D11Model>(node->GetModel()) });
	}

	m_transform = std::make_unique<D3D11VertexConstantBuffer<glm::mat4>>(*m_gfx);
}

void D3D11SampleRenderer::DrawFrame()
{
	auto cam = Engine::GetWorld()->GetActiveCamera();
	CLOG_WARN(!cam, "Renderer failed to find camera.");

	auto vp = cam->GetViewProjectionMatrix();


	m_gfx->ClearBuffer({ 0.f, 0.f, 0.f, 1.f });

	m_transform->Bind();
	for (auto sceneModel : m_sceneModels) {

		m_transform->Update(glm::transpose(vp * sceneModel.transform));


		for (auto& vgg : sceneModel.model->geometryGroups) {
			vgg->Draw();
		}
	}


	if (Engine::IsEditorActive()) {
		ImguiImpl::RenderD3D11();
	}

	m_gfx->EndFrame();
}
