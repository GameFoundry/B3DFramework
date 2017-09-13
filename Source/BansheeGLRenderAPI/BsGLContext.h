//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsGLPrerequisites.h"

namespace bs { namespace ct
{
	/** @addtogroup GL
	 *  @{
	 */

	/**
	 * Abstract class that encapsulated an OpenGL context. Each platform should provide its own GLContext specialization.
	 */
	class GLContext
	{
	public:
		GLContext();
		virtual ~GLContext();

		/**	Activates the rendering context (all subsequent rendering commands will be executed on it). */
        virtual void setCurrent(const RenderWindow& window) = 0;

		/**	Deactivates the rendering context. Normally called just before setCurrent is called on another context. */
		virtual void endCurrent() = 0;

		/**	Releases the render context, freeing all of its resources. */
		virtual void releaseContext() {}
	};

	/** @} */
}}