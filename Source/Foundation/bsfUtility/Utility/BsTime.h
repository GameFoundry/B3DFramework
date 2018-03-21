//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * Manages all time related functionality.
	 * 			
	 * @note	Sim thread only unless where specified otherwise.
	 */
	class BS_UTILITY_EXPORT Time : public Module<Time>
	{
	public:
		Time();
		~Time();

		/**
		 * Gets the time elapsed since application start. Only gets updated once per frame.
		 *
		 * @return	The time since application start, in seconds.
		 */
		float getTime() const { return mTimeSinceStart; }

		/**
		 * Gets the time elapsed since application start. Only gets updated once per frame.
		 *
		 * @return	The time since application start, in miliseconds.
		 */
		UINT64 getTimeMs() const { return mTimeSinceStartMs; }

		/**
		 * Gets the time since last frame was executed. Only gets updated once per frame.
		 *
		 * @return	Time since last frame was executed, in seconds.
		 */
		float getFrameDelta() const { return mFrameDelta; }

		/**
		 * Returns the sequential index of the current frame. First frame is 0.
		 *
		 * @return	The current frame.
		 *
		 * @note	Thread safe, but only counts sim thread frames.
		 */
		UINT64 getFrameIdx() const { return mCurrentFrame.load(); }

		/**
		 * Returns the precise time since application start, in microseconds. Unlike other time methods this is not only 
		 * updated every frame, but will return exact time at the moment it is called.
		 * 		
		 * @return	Time in microseconds.
		 *
		 * @note	
		 * You will generally only want to use this for performance measurements and similar. Use non-precise methods in 
		 * majority of code as it is useful to keep the time value equal in all methods during a single frame.
		 */
		UINT64 getTimePrecise() const;

		/**
		 * Gets the time at which the application was started, counting from system start.
		 *
		 * @return	The time since system to application start, in milliseconds.
		 */
		UINT64 getStartTimeMs() const { return mAppStartTime; }

		/** @name Internal 
		 *  @{
		 */

		/** Called every frame. Should only be called by Application. */
		void _update();

		/** @} */

		/** Multiply with time in microseconds to get a time in seconds. */
		static const double MICROSEC_TO_SEC;
	private:
		float mFrameDelta = 0.0f; /**< Frame delta in seconds */
		float mTimeSinceStart = 0.0f; /**< Time since start in seconds */
		UINT64 mTimeSinceStartMs = 0u;

		UINT64 mAppStartTime = 0u; /**< Time the application started, in microseconds */
		UINT64 mLastFrameTime = 0u; /**< Time since last runOneFrame call, In microseconds */
		std::atomic<unsigned long> mCurrentFrame{0UL};

		Timer* mTimer;
	};

	/** Easier way to access the Time module. */
	BS_UTILITY_EXPORT Time& gTime();

	/** @} */
}
