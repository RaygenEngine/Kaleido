#include "pch.h"
#include "renderer/Renderer.h"

ObserverRenderer::ObserverRenderer()
{
	m_nodeAddedListener.BindMember(this, &ObserverRenderer::OnNodeAddedToWorld);
	m_nodeRemovedListener.BindMember(this, &ObserverRenderer::OnNodeRemovedFromWorld);
}

void ObserverRenderer::RemoveObserver(NodeObserverBase* ptr)
{
	// WIP
	//m_observers.erase(std::find_if(begin(m_observers), end(m_observers),
	//								[ptr](auto observerUnqPtr)
	//{
	//	return observerUnqPtr.get() == ptr;
	//}
	//));
}

void ObserverRenderer::OnNodeRemovedFromWorld(Node* node)
{
	for (auto& observer : m_observers)
	{
		if (observer->baseNode == node)
		{
			observer->onObserveeLost(observer.get());
		}
	}
}

void ObserverRenderer::OnNodeAddedToWorld(Node* node)
{

}

void ObserverRenderer::Update()
{
	for (auto& observer : m_observers)
	{
		if (observer->baseNode)
		{
			auto flagset = observer->baseNode->GetDirtyFlagset();
			if (flagset.any())
			{
				observer->DirtyNodeUpdate(flagset);
			}
		}
	}
}
