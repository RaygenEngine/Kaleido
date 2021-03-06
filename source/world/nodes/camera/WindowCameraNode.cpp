#include "pch/pch.h"

#include "world/nodes/camera/WindowCameraNode.h"
#include "platform/windows/Win32Window.h"

WindowCameraNode::WindowCameraNode()
	: CameraNode()
{
	m_resizeListener.BindMember(this, &WindowCameraNode::WindowResize);
	auto mainWindow = Engine::GetMainWindow();

	if (mainWindow && mainWindow->GetWidth() > 0 && mainWindow->GetHeight() > 0) {
		m_viewportWidth = mainWindow->GetWidth();
		m_viewportHeight = mainWindow->GetHeight();
	}
}

void WindowCameraNode::WindowResize(int32 width, int32 height)
{
	m_viewportWidth = width;
	m_viewportHeight = height;
	SetDirty(DF::ViewportSize);
}
