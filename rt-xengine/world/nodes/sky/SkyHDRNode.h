#ifndef SKYHDRNODE_H
#define SKYHDRNODE_H

#include "world/nodes/Node.h"
#include "assets/texture/Texture.h"

namespace World
{
	class SkyHDRNode : public Node
	{
		std::shared_ptr<Assets::Texture> m_hdrTexture;

	public:
		SkyHDRNode(Node* parent);
		~SkyHDRNode() = default;

		bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

		Assets::Texture* GetSkyHDR() const { return m_hdrTexture.get(); }

	protected:
		std::string ToString(bool verbose, uint depth) const override;
	};
}

#endif // SKYHDRNODE_H
