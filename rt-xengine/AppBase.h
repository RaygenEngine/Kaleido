#ifndef APPBASE_H
#define APPBASE_H

// Base class for the custom and default "Game" class.
// Game class provides overrides for the most important classes, functions and settings of the base engine.

#include "world/NodeFactory.h"
#include "renderer/renderers/opengl/test/GLTestRenderer.h"
#include "system/Engine.h"
#include "platform/Window.h"

class AppBase
{
protected:
	std::string m_name;
	std::string m_initialScene;
	std::string m_assetPath;


	std::string m_windowTitle;
	int32 m_windowHeight;
	int32 m_windowWidth;

	bool m_handleControllers;

	bool m_lockMouse;
public:
	AppBase();

	virtual ~AppBase() = default;

public:
	virtual void PreMainInit(int32 argc, char* argv[]);
	virtual int32 Main(int32 argc, char* argv[]);

	
	virtual void RegisterRenderers(System::Engine* engine);

	virtual std::unique_ptr<System::Engine> CreateEngine();

	virtual std::unique_ptr<Platform::Window> CreateAppWindow(System::Engine* engine);

	virtual World::NodeFactory* MakeNodeFactory();

};


#endif //APPBASE_H