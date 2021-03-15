#pragma once

#include "renderer/renderers/contextual/d3d11/renderable/D3D11Renderable.h"

class D3D11Graphics;

struct GeometryGroup;
struct MaterialPod;


class D3D11GeometryGroup : public D3D11Renderable<D3D11GeometryGroup> {
public:
	D3D11GeometryGroup(D3D11Graphics& gfx, const GeometryGroup& gg, const MaterialPod& material);
};
