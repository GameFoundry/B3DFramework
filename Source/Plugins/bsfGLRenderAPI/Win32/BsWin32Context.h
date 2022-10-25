//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Win32/BsWin32Prerequisites.h"
#include "BsGLContext.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Windows specific implementation of an OpenGL context. */
		class Win32Context : public GLContext
		{
		public:
			/**
			 * Constructs a new context from a Windows device context and OpenGL rendering context. Optionally you may specify
			 * that the context isn't owned by us (might be created by some external library), in which case it will not be
			 * automatically released.
			 */
			Win32Context(HDC hdc, HGLRC glrc, bool ownsContext);
			virtual ~Win32Context();

			/** @copydoc GLContext::setCurrent */
			void SetCurrent(const RenderWindow& window) override;

			/** @copydoc GLContext::endCurrent */
			void EndCurrent() override;

			/** @copydoc GLContext::releaseContext  */
			void ReleaseContext() override;

		protected:
			HDC mHDC;
			HGLRC mGlrc;
			bool mOwnsContext;
		};

		/** @} */
	} // namespace ct
} // namespace bs
