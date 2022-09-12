//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGLRenderWindowManager.h"
#include "BsGLRenderAPI.h"
#include "BsGLSupport.h"
#include "Threading/BsAsyncOp.h"

namespace bs
{
	GLRenderWindowManager::GLRenderWindowManager(ct::GLRenderAPI* renderSystem)
		:mRenderSystem(renderSystem)
	{
		assert(mRenderSystem != nullptr);
	}

	SPtr<RenderWindow> GLRenderWindowManager::createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId,
		const SPtr<RenderWindow>& parentWindow)
	{
		ct::GLSupport* glSupport = mRenderSystem->GetGLSupport();

		// Create the window
		return glSupport->NewWindow(desc, windowId, parentWindow);
	}
}
