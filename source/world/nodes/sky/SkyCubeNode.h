#pragma once

#include "world/nodes/Node.h"
#include "asset/loaders/CubemapLoader.h"

class SkyCubeNode : public Node
{
	PodHandle<TexturePod> m_cubemap;

public:
	SkyCubeNode(Node* parent)
		: Node(parent) {}

	~SkyCubeNode() = default;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	PodHandle<TexturePod> GetSkyMap() const { return m_cubemap; }

protected:
	std::string ToString(bool verbose, uint depth) const override;

public:

	void ToString(std::ostream& os) const override { os << "node-type: SkyCubeNode, name: " << GetName(); }
};
