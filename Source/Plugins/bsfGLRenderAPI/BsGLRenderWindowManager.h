//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsRenderWindowManager.h"

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
		SPtr<RenderWindow> CreateImpl(RENDER_WINDOW_DESC& desc, u32 windowId, const SPtr<RenderWindow>& parentWindow) override;

	private:
		ct::GLRenderAPI* mRenderSystem;
	};

	/** @} */
} // namespace bs
