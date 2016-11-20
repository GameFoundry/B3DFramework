//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Utility-Core
	 *  @{
	 */

	/**
	 * Utility class for generating universally unique identifiers.
	 *
	 * @note	Thread safe.
	 */
	class BS_CORE_EXPORT UUIDGenerator
	{
	public:
		/**	Generate a new random universally unique identifier. */
		static String generateRandom();
	};

	/** @} */
}