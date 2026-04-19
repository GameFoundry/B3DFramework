//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Script/B3DIScriptExportable.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup Time
	 *  @{
	 */

	/**
	 * Manages global time related functionality.
	 *
	 * @note	Main thread only unless where specified otherwise.
	 */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(General)) Time : public Module<Time>
	{
	public:
		Time();
		~Time();

		/**
		 * Gets the time elapsed since application start. Only gets updated once per frame.
		 *
		 * @return	The time since application start, in seconds. This is real time, unaffected by simulation time scale.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RealTimeInSeconds))
		float GetRealTimeInSeconds() const { return mTimeSinceStart; }

		/**
		 * Gets the time elapsed since application start. Only gets updated once per frame.
		 *
		 * @return	The time since application start, in miliseconds. This is real time, unaffected by simulation time scale.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(RealTimeInMilliseconds))
		u64 GetRealTimeInMilliseconds() const { return mTimeSinceStartMs; }

		/**
		 * Gets the time since last frame was executed. Only gets updated once per frame.
		 *
		 * @return	Time since last frame was executed, in seconds.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(FrameDelta))
		float GetFrameDelta() const { return mFrameDelta; }

		/** Returns the time (in seconds) the latest frame has started. */
		float GetLastFrameTime() const { return (float)(mLastFrameTime * kMicrosecToSec); }

		/**
		 * Returns the sequential index of the current frame. First frame is 0.
		 *
		 * @return	The current frame.
		 *
		 * @note	Thread safe, but only counts main thread frames.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(CurrentFrameIndex))
		u64 GetCurrentFrameIndex() const { return mCurrentFrame.load(); }

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
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(TimePrecise))
		u64 GetTimePrecise() const;

		/**
		 * Gets the time at which the application was started, counting from system start.
		 *
		 * @return	The time since system to application start, in milliseconds.
		 */
		u64 GetStartTimeMs() const { return mAppStartTime; }

		/**
		 * Gets the current date and time in textual form.
		 *
		 * @param	isUTC	Outputs the date and time in Coordinated Universal Time, otherwise in local time.
		 *
		 * @return	A String containing the current date and time.
		 *
		 * @note
		 * Thread safe.
		 * The output format is [DayOfWeek], [Month] [NumericalDate], [NumericalYear] [HH]::[MM]::[SS].
		 */
		String GetCurrentDateTimeString(bool isUTC);

		/**
		 * Gets the current time in textual form
		 *
		 * @param	isUTC	Outputs the time in Coordinated Universal Time, otherwise in local time.
		 *
		 * @return	A String containing the current time.
		 *
		 * @note
		 * Thread safe.
		 * The output format is [HH]::[MM]::[SS].
		 */
		String GetCurrentTimeString(bool isUTC);

		/**
		 * Gets the date and time where the application has been started in textual form.
		 *
		 * @param	isUTC	Outputs the date and time in Coordinated Universal Time, otherwise in local time.
		 *
		 * @return	A String containing the application startup date and time.
		 *
		 * @note
		 * Thread safe.
		 * The output format is [DayOfWeek], [Month] [NumericalDate], [NumericalYear] [HH]::[MM]::[SS].
		 */
		String GetAppStartUpDateString(bool isUTC);

		/**
		 * Sets a fixed delta time for deterministic simulation. When enabled, GetFrameDelta() returns
		 * this constant value regardless of real-time elapsed.
		 *
		 * @param deltaSeconds Fixed time step in seconds (e.g., 0.016666 for 60 FPS). 0 disables.
		 */
		void SetFixedDeltaTime(float deltaSeconds);

		/** Returns the fixed delta time in microseconds, or 0 if using real-time. */
		u64 GetFixedDeltaTimeUs() const { return mFixedDeltaTimeMicrosec; }

		/** @name Internal
		 *  @{
		 */

		/** Called every frame. Should only be called by Application. */
		void Update();

		/** @} */

		/** Multiply with time in microseconds to get a time in seconds. */
		static const double kMicrosecToSec;

	private:
		float mFrameDelta = 0.0f; /**< Frame delta in seconds */
		float mTimeSinceStart = 0.0f; /**< Time since start in seconds */
		u64 mTimeSinceStartMs = 0u;
		bool mFirstFrame = true;
		u64 mFixedDeltaTimeMicrosec = 0; /**< Fixed delta time in microseconds (0 = disabled, use real-time) */

		u64 mAppStartTime = 0u; /**< Time the application started, in microseconds */
		u64 mLastFrameTime = 0u; /**< Time since last runOneFrame call, In microseconds */
		std::atomic<unsigned long> mCurrentFrame{ 0UL };

		std::time_t mAppStartUpDate;

		Timer* mTimer;
	};

	/** Easier way to access the Time module. */
	B3D_EXPORT Time& GetTime();

	/** Manages simulation time for a particular scene. This time runs only while scene is simulating. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(General)) SceneTime : public IScriptExportable
	{
	public:
		SceneTime();
		~SceneTime();

		/**
		 * Gets the time since the simulation started playing, multiplied by the time scale factor. In editor this will reset to zero every time you
		 * start playing in editor, and in a standalone application this will be similar to GetRealTimeInSeconds(), except simulation time can be
		 * sped up/down, or stopped entirely by setting the time scale.
		 *
		 * @return	Time since game start, affected by simulation time scale.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(TimeInSeconds))
		float GetTimeInSeconds() const { return mTimeInSeconds; }

		/** Allows you to speed time up or down, or completely pause it by providing zero. Must be zero or larger. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(Scale))
		void SetScale(float scale);

		/** Returns the currently applied simulation time scale. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Scale))
		float GetScale() const { return mTimeScale; }

		/** Resets the simulation time to zero. Primarily used for editor purposes for resetting the time when ending play in editor. */
		B3D_SCRIPT_EXPORT()
		void Reset() { mTimeInSeconds = 0.0f; }

		/** Pauses or unpauses the simulation time. This is equivalent to setting the time scale to 0. */
		B3D_SCRIPT_EXPORT()
		void SetPaused(bool paused) { mIsTimePaused = paused;}

		/**
		 * Sets a fixed delta time for deterministic simulation. When enabled, the base frame delta
		 * uses this constant value before time scale is applied.
		 *
		 * @param delta		Fixed time step in microseconds. 0 disables.
		 */
		void SetFixedDeltaTimeUs(u64 delta);

		/** Returns the step (in seconds) between fixed frame updates. */
		float GetFixedFrameDelta() const { return (float)(mFixedStep * Time::kMicrosecToSec); }

		/** Returns the time (in seconds) the latest fixed update has started. */
		float GetLastFixedUpdateTime() const { return (float)(mLastFixedUpdateTime * Time::kMicrosecToSec); }

		/** @name Internal
		 *  @{
		 */

		/** Called every frame. Should only be called by Application. */
		void Update();

		/**
		 * Calculates the number of fixed update iterations required and their step size. Values depend on the current
		 * time and previous calls to AdvanceFixedUpdateInternal().;
		 *
		 * @param	outStep	Duration of the fixed step in microseconds. In most cases this is the same duration as
		 *					the	fixed time delta, but in the cases where frame is taking a very long time the step
		 *					might be increased to avoid a large number of fixed updates per frame.
		 * @return			Returns the number of fixed frame updates to execute (each of @p outStep duration). In most
		 *					cases this will be either 1 or 0, or a larger amount of frames are taking a long time
		 *					to execute (longer than a multiple of fixed frame step).
		 */
		u32 GetFixedUpdateStep(u64& outStep);

		/**
		 * Advances the fixed update timers by @p step microseconds. Should be called once for each iteration as returned
		 * by GetFixedUpdateStepInternal(), per frame.
		 */
		void AdvanceFixedUpdate(u64 step);

		/** @} */

	private:
		/** Maximum number of fixed updates that can ever be accumulated. */
		static constexpr u32 kMaximumAccumulatedFixedUpdates = 200;

		/** Determines how many new fixed updates are regenerated per frame. */
		static constexpr u32 kNewFixedUpdatesPerFrame = 4;

		float mFrameDelta = 0.0f; /**< Frame delta in seconds */
		bool mFirstFrame = true;

		u64 mLastFrameTime = 0u; /**< Time since last frame update call, In microseconds */

		float mTimeInSeconds = 0.0f;
		float mTimeScale = 1.0f;
		bool mIsTimePaused = false;

		// Fixed update
		u64 mFixedStep = 16666; // 60 times a second in microseconds
		u64 mLastFixedUpdateTime = 0;
		bool mFirstFixedFrame = true;
		u32 mRemainingFixedUpdateCount = kMaximumAccumulatedFixedUpdates;

		u64 mFixedDeltaTimeMicrosec = 0; /**< Fixed delta time in microseconds (0 = disabled, use real-time) */

		Timer* mTimer;
	};

	/** @} */
} // namespace b3d
