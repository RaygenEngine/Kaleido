#pragma once

#include "editor/Editor.h"
class Node;

class PropertyEditor {
public:
	bool m_localMode{ true };
	bool m_displayMatrix{ false };
	bool m_massEditMaterials{ false };

	// Injects the imgui code of a property editor from a node.
	void Inject(Node* node);

	void Run_BaseProperties(Node* node);

	void Run_ContextActions(Node* node);

	void Run_ReflectedProperties(Node* node);
};
