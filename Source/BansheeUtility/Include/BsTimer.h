//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisitesUtil.h"

namespace BansheeEngine
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Timer class used for querying high precision timers.
	 *
	 * @note	Not thread safe.
	 */
	class BS_UTILITY_EXPORT Timer
    {
    public:
		/** Construct the timer and start timing. */
		Timer();

		/** Reset the timer to zero. */
		void reset();

		/** Returns time in milliseconds since timer was initialized or last reset. */
		UINT64 getMilliseconds() const;

		/** Returns time in microseconds since timer was initialized or last reset. */
		UINT64 getMicroseconds() const;

		/**
		 * Returns the time at which the timer was initialized, in milliseconds.
		 *
		 * @return	Time in milliseconds.
		 */
		UINT64 getStartMs() const;

	private:
		std::chrono::high_resolution_clock mHRClock;
		std::chrono::steady_clock::time_point mStartTime;
    };

	/** @} */
}
