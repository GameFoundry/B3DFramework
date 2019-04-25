//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLContext.h"
#include <GL/glxew.h>

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**	Linux specific implementation of an OpenGL context. */
	class LinuxContext : public GLContext
	{
	public:
		/**
		 * Constructs a new OpenGL context.
		 *
		 * @param[in] 	x11display 		X11 display all windows using this context will be created with.
		 * @param[in]	visualInfo		X11 visual info describing properties of all windows that will use this context.
		 **/
		LinuxContext(::Display* x11display, XVisualInfo& visualInfo);
		virtual ~LinuxContext();

		/** @copydoc GLContext::setCurrent */
		void setCurrent(const RenderWindow& window) override;

		/** @copydoc GLContext::endCurrent */
		void endCurrent() override;

		/** @copydoc GLContext::releaseContext  */
		void releaseContext() override;

	protected:
		::Display* mDisplay;
		::Window mCurrentWindow = 0;
		GLXContext mContext = 0;
	};

	/** @} */
}}