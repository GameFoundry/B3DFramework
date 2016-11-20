//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLRenderAPI.h"
#include "BsRenderWindow.h"

namespace bs
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * Helper class dealing mostly with platform specific OpenGL functionality, initialization, extensions and window
	 * creation.
	 */
	class BS_RSGL_EXPORT GLSupport
	{
	public:
		GLSupport() { }
		virtual ~GLSupport() { }

		/**
		 * Creates a new render window using the specified descriptor.
		 *
		 * @param[in]	desc			Description of a render window to create.
		 * @param[in]	windowId		Window ID provided by the render window manager.
		 * @param[in]	parentWindow	Optional parent window if the window shouldn't be a main window. First created
		 *								window cannot have a parent.
		 * @return						Returns newly created window.
		 */
		virtual SPtr<RenderWindow> newWindow(RENDER_WINDOW_DESC& desc, UINT32 windowId, SPtr<RenderWindow> parentWindow) = 0;

		/**
		 * Creates a new render window using the specified descriptor.
		 *
		 * @param[in]	desc			Description of a render window to create.
		 * @param[in]	windowId		Window ID provided by the render window manager.
		 * @return						Returns newly created window.
		 */
		virtual SPtr<RenderWindowCore> newWindowCore(RENDER_WINDOW_DESC& desc, UINT32 windowId) = 0;

		/**	Called when OpenGL is being initialized. */
		virtual void start() = 0;

		/**	Called when OpenGL is being shut down. */
		virtual void stop() = 0;

		/**	Gets OpenGL vendor name. */
		const String& getGLVendor() const
		{
			return mVendor;
		}

		/**	Gets OpenGL version string. */
		const String& getGLVersion() const
		{
			return mVersion;
		}

		/**	Checks is the specified extension available. */
		virtual bool checkExtension(const String& ext) const;

		/**	Gets an address of an OpenGL procedure with the specified name. */
		virtual void* getProcAddress(const String& procname) = 0;

		/** Initializes OpenGL extensions. Must be called after we have a valid and active OpenGL context. */
		virtual void initializeExtensions();

		/**	Gets a structure describing all available video modes. */
		virtual SPtr<VideoModeInfo> getVideoModeInfo() const = 0;

	protected:
		Set<String> extensionList;

		String mVersion;
		String mVendor;
	};

	/** @} */
};