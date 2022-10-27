//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLRenderWindowManager.h"
#include "BsGLRenderAPI.h"
#include "BsGLSupport.h"
#include "Threading/BsAsyncOp.h"

using namespace bs;

GLRenderWindowManager::GLRenderWindowManager(ct::GLRenderAPI* renderSystem)
	: mRenderSystem(renderSystem)
{
	assert(mRenderSystem != nullptr);
}

SPtr<RenderWindow> GLRenderWindowManager::CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow)
{
	ct::GLSupport* glSupport = mRenderSystem->GetGlSupport();

	// Create the window
	return glSupport->NewWindow(desc, windowId, parentWindow);
}
