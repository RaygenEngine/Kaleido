#pragma once

#include "world/nodes/Node.h"

// Window dependant // TODO: non-window dependant (e.g. oculus virtual eye)
class CameraNode : public Node
{
	// distance to film plane
	float m_focalLength;

	// vertical fov (full angle)
	float m_vFov;
	// horizontal fov depends on the vertical and the aspect ratio
	float m_hFov;

	float m_near;
	float m_far;

	glm::mat4 m_projectionMatrix;

public:
	CameraNode(Node* parent);
	~CameraNode() = default;

	glm::vec3 GetLookAt() const { return GetWorldTranslation() + (GetWorldOrientation() * glm::vec3(0.f, 0.f, -1.f)) * m_focalLength; }

	float GetVerticalFov() const { return m_vFov; }
	float GetHorizontalFov() const { return m_hFov; }
	float GetFocalLength() const { return m_focalLength; }

	float GetNear() const { return m_near; }
	float GetFar() const { return m_far; }

	std::string ToString(bool verbose, uint depth) const override;

	glm::mat4 GetViewMatrix() const { return glm::lookAt(GetWorldTranslation(), GetLookAt(), GetUp()); }
	glm::mat4 GetProjectionMatrix() const { return m_projectionMatrix; }
		
	void GetTracingVariables(glm::vec3& u, glm::vec3& v, glm::vec3& w);

	// REFACT: 
	//void WindowResize(int32 width, int32 height) override;

	bool LoadAttributesFromXML(const tinyxml2::XMLElement* xmlData) override;

	void ToString(std::ostream& os) const override { os << "node-type: CameraNode, name: " << m_name; }
};

