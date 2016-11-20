//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Material-Internal
	 *  @{
	 */

	/**	Allows you to specify defines that can control shader compilation. */
	class BS_CORE_EXPORT ShaderDefines
	{
	public:
		/** Adds a new define with a floating point value. */
		void set(const String& name, float value);

		/** Adds a new define with an integer value. */
		void set(const String& name, INT32 value);

		/** Adds a new define with an integer value. */
		void set(const String& name, UINT32 value);

		/** Adds a new define with a string point value. */
		void set(const String& name, const String& value);

		/**	Returns a list of all defines. */
		UnorderedMap<String, String> getAll() const { return mDefines; }

		/** Removes all defines. */
		void clear() { mDefines.clear(); }

	protected:
		UnorderedMap<String, String> mDefines;
	};

	/** @} */
}