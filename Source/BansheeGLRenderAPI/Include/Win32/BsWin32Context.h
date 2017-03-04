//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsWin32Prerequisites.h"
#include "BsGLContext.h"

namespace bs { namespace ct
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
        void setCurrent() override;

        /** @copydoc GLContext::endCurrent */
		void endCurrent() override;

        /** @copydoc GLContext::releaseContext  */
		void releaseContext() override;

	protected:
        HDC     mHDC;
        HGLRC   mGlrc;
		bool	mOwnsContext;
    };

	/** @} */
}}