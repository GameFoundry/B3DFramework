//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGLRenderWindowManager.h"
#include "BsGLRenderAPI.h"
#include "BsGLSupport.h"
#include "BsAsyncOp.h"

namespace bs
{
	GLRenderWindowManager::GLRenderWindowManager(GLRenderAPI* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	SPtr<RenderWindow> GLRenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, const SPtr<RenderWindow>& parentWindow)
	{
		GLSupport* glSupport = mRenderSystem->getGLSupport();

		// Create the window
		return glSupport->newWindow(desc, windowId, parentWindow);
	}

	GLRenderWindowCoreManager::GLRenderWindowCoreManager(GLRenderAPI* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	SPtr<RenderWindowCore> GLRenderWindowCoreManager::createInternal(RENDER_WINDOW_DESC& desc, UINT32 windowId)
	{
		GLSupport* glSupport = mRenderSystem->getGLSupport();

		// Create the window
		SPtr<RenderWindowCore> window = glSupport->newWindowCore(desc, windowId);
		window->_setThisPtr(window);

		windowCreated(window.get());

		return window;
	}
}