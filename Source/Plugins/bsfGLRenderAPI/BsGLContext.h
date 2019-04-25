//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
		GLContext() = default;
		virtual ~GLContext() = default;

		/**	Activates the rendering context (all subsequent rendering commands will be executed on it). */
		virtual void setCurrent(const RenderWindow& window) = 0;

		/**	Deactivates the rendering context. Normally called just before setCurrent is called on another context. */
		virtual void endCurrent() = 0;

		/**	Releases the render context, freeing all of its resources. */
		virtual void releaseContext() {}
	};

	/** @} */
}}