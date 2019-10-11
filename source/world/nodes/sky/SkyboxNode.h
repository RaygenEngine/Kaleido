#pragma once

#include "world/nodes/Node.h"
#include "asset/pods/TexturePod.h"

class SkyboxNode : public Node {
	REFLECTED_NODE(SkyboxNode, Node) { REFLECT_VAR(m_cubemap); }

	PodHandle<TexturePod> m_cubemap;

public:
	[[nodiscard]] PodHandle<TexturePod> GetSkyMap() const { return m_cubemap; }
};
