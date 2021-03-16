#include "D3D11Model.h"

#include "renderer/renderers/contextual/d3d11/renderable/D3D11GeometryGroup.h"
#include "asset/AssetManager.h"

void D3D11Model::Load(D3D11Graphics& gfx)
{
	auto modelData = podHandle.Lock();
	timer::ScopedTimer<ch::milliseconds> _("Uploading model time");

	for (auto m : modelData->meshes) {
		for (auto gg : m.geometryGroups) {

			auto mat = modelData->materials.at(gg.materialIndex).Lock();

			geometryGroups.emplace_back(std::make_unique<D3D11GeometryGroup>(gfx, gg, *mat));
		}
	}
}
