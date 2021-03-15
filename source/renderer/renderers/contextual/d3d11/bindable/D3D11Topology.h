#pragma once

#include "renderer/renderers/contextual/d3d11/bindable/D3D11Bindable.h"

#include <d3d11.h>

class D3D11Graphics;

class D3D11Topology : public D3D11Bindable {

public:
	D3D11Topology(D3D11Graphics& gfx, D3D11_PRIMITIVE_TOPOLOGY type);
	void Bind() override;

private:
	D3D11_PRIMITIVE_TOPOLOGY m_type;
};

#include "asset/pods/ModelPod.h"

inline D3D11_PRIMITIVE_TOPOLOGY GetTopologyFromEngine(GeometryMode mode)
{
	switch (mode) {
		case GeometryMode::POINTS: return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
		case GeometryMode::LINE: return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
		case GeometryMode::LINE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
		case GeometryMode::TRIANGLES: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
		case GeometryMode::TRIANGLE_STRIP: return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;

		case GeometryMode::LINE_LOOP:
		case GeometryMode::TRIANGLE_FAN:
		default: return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
	}
}
