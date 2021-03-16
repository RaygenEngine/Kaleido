#pragma once

#include "renderer/renderers/contextual/d3d11/asset/D3D11Asset.h"
#include "asset/pods/ModelPod.h"

class D3D11GeometryGroup;

struct D3D11Model : D3D11Asset<ModelPod> {

	std::vector<std::unique_ptr<D3D11GeometryGroup>> geometryGroups;

	D3D11Model(PodHandle<ModelPod> handle)
		: D3D11Asset(handle)
	{
	}

protected:
	void Load(D3D11Graphics& gfx) override;
};
