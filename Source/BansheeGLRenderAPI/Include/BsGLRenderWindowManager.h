//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsRenderWindowManager.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Manager that handles window creation for OpenGL. */
	class GLRenderWindowManager : public RenderWindowManager
	{
	public:
		GLRenderWindowManager(ct::GLRenderAPI* renderSystem);

	protected:
		/** @copydoc RenderWindowManager::createImpl() */
		SPtr<RenderWindow> createImpl(RENDER_WINDOW_DESC& desc, UINT32 windowId, const SPtr<RenderWindow>& parentWindow) override;

	private:
		ct::GLRenderAPI* mRenderSystem;
	};

	namespace ct
	{
	/**	Manager that handles window creation for OpenGL. */
	class GLRenderWindowManager : public RenderWindowManager
	{
	public:
		GLRenderWindowManager(GLRenderAPI* renderSystem);

	protected:
		/** @copydoc RenderWindowManager::createInternal */
		SPtr<RenderWindow> createInternal(RENDER_WINDOW_DESC& desc, UINT32 windowId) override;

	private:
		GLRenderAPI* mRenderSystem;
	};
	}

	/** @} */
}