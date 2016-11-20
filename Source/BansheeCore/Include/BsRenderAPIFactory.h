//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/** Factory class that you may specialize in order to start up a specific render system. */
	class RenderAPIFactory
	{
	public:
		virtual ~RenderAPIFactory() { }

		/**	Creates and starts up the render system managed by this factory.  */
		virtual void create() = 0;

		/**	Returns the name of the render system this factory creates. */
		virtual const char* name() const = 0;
	};

	/** @} */
}