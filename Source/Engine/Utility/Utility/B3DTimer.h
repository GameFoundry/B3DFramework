//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"

namespace b3d
{
	/** @addtogroup General
	 *  @{
	 */

	/** Timer class used for querying high precision timers. */
	class B3D_EXPORT Timer
	{
	public:
		/** Construct the timer and start timing. */
		Timer();

		/** Reset the timer to zero. */
		void Reset();

		/** Returns time in milliseconds since timer was initialized or last reset. */
		u64 GetMilliseconds() const;

		/** Returns time in microseconds since timer was initialized or last reset. */
		u64 GetMicroseconds() const;

		/**
		 * Returns the time at which the timer was initialized, in milliseconds.
		 *
		 * @return	Time in milliseconds.
		 */
		u64 GetStartMs() const;

	private:
		std::chrono::high_resolution_clock mHRClock;
		std::chrono::time_point<std::chrono::high_resolution_clock> mStartTime;
	};

	/** @} */
} // namespace b3d
