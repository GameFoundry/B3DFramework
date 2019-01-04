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

		/** Returns the step (in seconds) between fixed frame updates. */
		float getFixedFrameDelta() const { return (float)(mFixedStep * MICROSEC_TO_SEC); }

		/** Returns the time (in seconds) the latest frame has started. */
		float getLastFrameTime() const { return (float)(mLastFrameTime * MICROSEC_TO_SEC); }

		/** Returns the time (in seconds) the latest fixed update has started. */
		float getLastFixedUpdateTime() const { return (float)(mLastFixedUpdateTime * MICROSEC_TO_SEC); }

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

		/**
		 * Gets the current date and time in textual form.
		 * 
		 * @param[in]	isUTC Outputs the date and time in Coordinated Universal Time, otherwise in local time.
		 * 
		 * @return	A String containing the current date and time.
		 * 
		 * @note
		 * Thread safe.
		 * The output format is [DayOfWeek], [Month] [NumericalDate], [NumericalYear] [HH]::[MM]::[SS].
		 */
		String getCurrentDateTime(bool isUTC);

		/**
		 * Gets the current time in textual form
		 * 
		 * @param[in]	isUTC Outputs the time in Coordinated Universal Time, otherwise in local time.
		 * 
		 * @return	A String containing the current time.
		 * 
		 * @note
		 * Thread safe.
		 * The output format is [HH]::[MM]::[SS].
		 */
		String getCurrentTime(bool isUTC);

		/**
		 * Gets the date and time where the application has been started in textual form.
		 *
		 * @param[in]	isUTC Outputs the date and time in Coordinated Universal Time, otherwise in local time.
		 * 
		 * @return	A String containing the application startup date and time.
		 * 
		 * @note
		 * Thread safe.
		 * The output format is [DayOfWeek], [Month] [NumericalDate], [NumericalYear] [HH]::[MM]::[SS].
		 */
		String getAppStartUpDate(bool isUTC);


		/** @name Internal 
		 *  @{
		 */

		/** Called every frame. Should only be called by Application. */
		void _update();

		/** 
		 * Calculates the number of fixed update iterations required and their step size. Values depend on the current
		 * time and previous calls to _advanceFixedUpdate().;
		 * 
		 * @param[out]		step	Duration of the fixed step in microseconds. In most cases this is the same duration as
		 *							the	fixed time delta, but in the cases where frame is taking a very long time the step
		 *							might be increased to avoid a large number of fixed updates per frame.
		 * @return					Returns the number of fixed frame updates to execute (each of @p step duration). In most
		 *							cases this will be either 1 or 0, or a larger amount of frames are taking a long time
		 *							to execute (longer than a multiple of fixed frame step). 
		 */
		UINT32 _getFixedUpdateStep(UINT64& step);

		/** 
		 * Advances the fixed update timers by @p step microseconds. Should be called once for each iteration as returned
		 * by _getFixedUpdateStep(), per frame. 
		 */
		void _advanceFixedUpdate(UINT64 step);

		/** @} */

		/** Multiply with time in microseconds to get a time in seconds. */
		static const double MICROSEC_TO_SEC;
	private:
		float mFrameDelta = 0.0f; /**< Frame delta in seconds */
		float mTimeSinceStart = 0.0f; /**< Time since start in seconds */
		UINT64 mTimeSinceStartMs = 0u;
		bool mFirstFrame = true;

		UINT64 mAppStartTime = 0u; /**< Time the application started, in microseconds */
		UINT64 mLastFrameTime = 0u; /**< Time since last runOneFrame call, In microseconds */
		std::atomic<unsigned long> mCurrentFrame{0UL};

		// Fixed update
		UINT64 mFixedStep = 16666; // 60 times a second in microseconds
		UINT64 mLastFixedUpdateTime = 0;
		bool mFirstFixedFrame = true;

		std::time_t mAppStartUpDate;

		Timer* mTimer;
	};

	/** Easier way to access the Time module. */
	BS_UTILITY_EXPORT Time& gTime();

	/** @} */
}
