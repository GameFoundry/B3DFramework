//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "BsGLRenderAPI.h"
#include "RenderAPI/BsRenderWindow.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**
		 * Helper class dealing mostly with platform specific OpenGL functionality, initialization, extensions and window
		 * creation.
		 */
		class GLSupport
		{
		public:
			GLSupport() = default;
			virtual ~GLSupport() = default;

			/**
			 * Creates a new render window using the specified descriptor.
			 *
			 * @param[in]	desc			Description of a render window to create.
			 * @param[in]	windowId		Window ID provided by the render window manager.
			 * @param[in]	parentWindow	Optional parent window if the window shouldn't be a main window. First created
			 *								window cannot have a parent.
			 * @return						Returns newly created window.
			 */
			virtual SPtr<bs::RenderWindow> NewWindow(RENDER_WINDOW_DESC& desc, u32 windowId, SPtr<bs::RenderWindow> parentWindow) = 0;

			/**	Called when OpenGL is being initialized. */
			virtual void Start() = 0;

			/**	Called when OpenGL is being shut down. */
			virtual void Stop() = 0;

			/**	Gets OpenGL vendor name. */
			const String& GetGlVendor() const
			{
				return mVendor;
			}

			/**	Gets OpenGL version string. */
			const String& GetGlVersion() const
			{
				return mVersion;
			}

			/**	Checks is the specified extension available. */
			virtual bool CheckExtension(const String& ext) const;

			/**	Gets an address of an OpenGL procedure with the specified name. */
			virtual void* GetProcAddress(const String& procname) = 0;

			/** Initializes OpenGL extensions. Must be called after we have a valid and active OpenGL context. */
			virtual void InitializeExtensions();

			/**	Gets a structure describing all available video modes. */
			virtual SPtr<VideoModeInfo> GetVideoModeInfo() const = 0;

		protected:
			Set<String> extensionList;

			String mVersion;
			String mVendor;
		};

		/** @} */
	} // namespace ct
} // namespace bs
