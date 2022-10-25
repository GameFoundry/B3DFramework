//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLSupport.h"
#include "BsGLRenderAPI.h"

namespace bs::ct
{
	class MacOSContext;

	/** @addtogroup GL
	 *  @{
	 */

	/**	Handles OpenGL initialization, window creation and extensions on MacOS. */
	class MacOSGLSupport : public GLSupport
	{
	public:
		/** @copydoc GLSupport::newWindow */
		SPtr<bs::RenderWindow> newWindow(RENDER_WINDOW_DESC& desc, u32 windowId, SPtr<bs::RenderWindow> parentWindow) override;

		/** @copydoc GLSupport::start */
		void start() override;

		/** @copydoc GLSupport::stop */
		void stop() override;

		/** @copydoc GLSupport::getProcAddress */
		void* getProcAddress(const String& procname) override;

		/** Creates a new OpenGL context. */
		SPtr<MacOSContext> createContext(bool depthStencil, u32 msaaCount);

		/** @copydoc GLSupport::getVideoModeInfo */
		SPtr<VideoModeInfo> getVideoModeInfo() const override;
	};

	/** @} */
} // namespace bs::ct
