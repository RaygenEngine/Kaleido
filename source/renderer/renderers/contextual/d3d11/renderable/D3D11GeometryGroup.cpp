#include "D3D11GeometryGroup.h"

#include "renderer/renderers/contextual/d3d11/bindable/D3D11InputLayout.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11VertexBuffer.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11VertexShader.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11PixelShader.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11IndexBuffer.h"
#include "renderer/renderers/contextual/d3d11/bindable/D3D11Topology.h"

#include "asset/pods/ModelPod.h"
#include "asset/pods/MaterialPod.h"

D3D11GeometryGroup::D3D11GeometryGroup(D3D11Graphics& gfx, const GeometryGroup& gg, const MaterialPod& material)
	: D3D11Renderable(gfx)
{
	if (!IsStaticInitialized()) {
		// WIP:

		auto pvs = std::make_unique<D3D11VertexShader>(
			m_gfx, L"C:/dev/Kaleido/assets/engine-data/hlsl/TestVS.hlsl"); // TODO: shader from material
		auto pvsbc = pvs->GetBytecode();
		AddStaticBind(std::move(pvs));

		AddStaticBind(std::make_unique<D3D11PixelShader>(m_gfx, L"C:/dev/Kaleido/assets/engine-data/hlsl/TestPS.hlsl"));

		// clang-format off
		const std::vector<D3D11_INPUT_ELEMENT_DESC> ied {
			{ "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Tangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "Bitangent", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UVf", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "UVs", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		// clang-format on

		AddStaticBind(std::make_unique<D3D11InputLayout>(m_gfx, ied, pvsbc));
	}

	AddBind(std::make_unique<D3D11Topology>(m_gfx, GetTopologyFromEngine(gg.mode)));
	AddBind(std::make_unique<D3D11VertexBuffer>(m_gfx, gg.vertices));
	AddIndexBuffer(std::make_unique<D3D11IndexBuffer>(m_gfx, gg.indices));
}
