//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2017 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
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
		SPtr<bs::RenderWindow> newWindow(RENDER_WINDOW_DESC& desc, UINT32 windowId, SPtr<bs::RenderWindow> parentWindow) override;

		/** @copydoc GLSupport::start */
		void start() override;

		/** @copydoc GLSupport::stop */
		void stop() override;

		/** @copydoc GLSupport::getProcAddress */
		void* getProcAddress(const String& procname) override;

		/** Creates a new OpenGL context. */
		SPtr<MacOSContext> createContext(bool depthStencil, UINT32 msaaCount);

		/** @copydoc GLSupport::getVideoModeInfo */
		SPtr<VideoModeInfo> getVideoModeInfo() const override;
	};

	/** @} */
}

