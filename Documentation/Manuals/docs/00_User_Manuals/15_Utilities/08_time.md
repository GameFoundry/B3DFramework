---
title: Measuring time
---

Being able to tell the current time, as well as elapsed time since the last frame is important for any real-time application. The framework provides two time systems: global time through @b3d::Time for application-wide timing, and scene time through @b3d::SceneTime for per-scene simulation timing.

# Global time

Use the @b3d::Time class, accessible through @b3d::GetTime(), to retrieve global information about time in the framework.

## Current time

Use @b3d::Time::GetRealTimeInSeconds() to get the current time (since application start) in seconds:

~~~~~~~~~~~~~{.cpp}
float curTime = GetTime().GetRealTimeInSeconds();
B3D_LOG(Info, LogGeneric, "Application was started {0} seconds ago.", curTime);
~~~~~~~~~~~~~

This value is only updated once per frame (i.e., it stays constant throughout a frame). For millisecond precision, use @b3d::Time::GetRealTimeInMilliseconds():

~~~~~~~~~~~~~{.cpp}
u64 curTimeMs = GetTime().GetRealTimeInMilliseconds();
B3D_LOG(Info, LogGeneric, "Application was started {0} ms ago.", curTimeMs);
~~~~~~~~~~~~~

## Precise time measurements

If you need more precise time that can be used for inter-frame measurements, use @b3d::Time::GetTimePrecise(), which returns the current time in microseconds:

~~~~~~~~~~~~~{.cpp}
u64 preciseTimeStart = GetTime().GetTimePrecise();

u64 counter = 0;
for (i32 i = 0; i < 1000000; i++)
    counter += i % 10;

u64 preciseTimeEnd = GetTime().GetTimePrecise();
u64 timeElapsed = preciseTimeEnd - preciseTimeStart;

float secondsElapsed = (float)(timeElapsed * Time::kMicrosecToSec);
B3D_LOG(Info, LogGeneric, "Operation took {0} seconds.", secondsElapsed);
~~~~~~~~~~~~~

> @b3d::Time::kMicrosecToSec is a constant to convert between microseconds and seconds.

You should use **GetRealTimeInSeconds()** for most gameplay purposes, while **GetTimePrecise()** can be used for profiling and similar situations where you need precise measurements.

## Frame delta time

Often it is useful to know how much time has passed since the last frame. Use @b3d::Time::GetFrameDelta() to get the elapsed time from the previous frame:

~~~~~~~~~~~~~{.cpp}
float elapsedTime = GetTime().GetFrameDelta();
B3D_LOG(Info, LogGeneric, "Last frame was {0} seconds ago.", elapsedTime);
~~~~~~~~~~~~~

> **Note:** This is the real-time frame delta, unaffected by scene time scale. For gameplay that should respect time scaling and pausing, use scene time instead (see below).

## Frame index

Sometimes, often for debugging purposes, it is useful to know the index of the current frame. Use @b3d::Time::GetCurrentFrameIndex(). Each frame the index gets incremented by one:

~~~~~~~~~~~~~{.cpp}
u64 frameIdx = GetTime().GetCurrentFrameIndex();
B3D_LOG(Info, LogGeneric, "This is frame #{0}", frameIdx);
~~~~~~~~~~~~~

## Date and time strings

You can get human-readable date and time strings:

~~~~~~~~~~~~~{.cpp}
// Get current date and time
String dateTime = GetTime().GetCurrentDateTimeString(false); // Local time
String dateTimeUTC = GetTime().GetCurrentDateTimeString(true); // UTC

// Get current time only
String timeStr = GetTime().GetCurrentTimeString(false);

// Get application startup date/time
String startupDate = GetTime().GetAppStartUpDateString(false);
~~~~~~~~~~~~~

# Scene time

Each scene instance has its own time management through the @b3d::SceneTime object. This allows you to control time independently for different scenes, enabling features like pause, slow motion, and time scaling.

## Accessing scene time

You can access scene time through a scene instance or from within a component:

~~~~~~~~~~~~~{.cpp}
// From scene instance
TShared<SceneInstance> sceneInstance = SceneInstance::Create("MyScene");
SceneTime& time = sceneInstance->GetTime();

// From within a component
class MyComponent : public Component
{
    void Update() override
    {
        SceneTime& time = SceneObject()->GetScene()->GetTime();
        // Use scene time...
    }
};
~~~~~~~~~~~~~

## Scene time properties

Scene time provides several key properties:

~~~~~~~~~~~~~{.cpp}
SceneTime& time = sceneInstance->GetTime();

// Get simulation time since scene started (affected by time scale)
float timeSinceStart = time.GetTimeInSeconds();

// Get current time scale
float scale = time.GetScale();

// Get fixed update delta (for physics)
float fixedDelta = time.GetFixedFrameDelta();
~~~~~~~~~~~~~

> Unlike global time, scene time is affected by time scale and can be paused. It resets to zero when the scene starts playing.

## Time scaling

Time scaling allows you to speed up, slow down, or pause simulation:

~~~~~~~~~~~~~{.cpp}
SceneTime& time = sceneInstance->GetTime();

// Pause time in the scene
time.SetScale(0.0f);

// Slow motion (half speed)
time.SetScale(0.5f);

// Fast forward (double speed)
time.SetScale(2.0f);

// Resume normal time
time.SetScale(1.0f);
~~~~~~~~~~~~~

Time scaling affects:
- Component Update() delta time
- Component FixedUpdate() timing
- Animation playback speed
- Physics simulation speed (if tied to scene time)
- Particle system updates

## Pausing simulation

You can explicitly pause/unpause simulation:

~~~~~~~~~~~~~{.cpp}
SceneTime& time = sceneInstance->GetTime();

// Pause the simulation
time.SetPaused(true);

// Resume the simulation
time.SetPaused(false);
~~~~~~~~~~~~~

> **Note:** Setting paused is equivalent to setting the time scale to 0, but is more explicit in intent.

## Fixed timestep

Scene time manages fixed timestep updates for physics and other systems that require consistent timing:

~~~~~~~~~~~~~{.cpp}
SceneTime& time = sceneInstance->GetTime();

// Get the fixed update delta (default is 1/60th of a second)
float fixedDelta = time.GetFixedFrameDelta();

// Get the time of the last fixed update
float lastFixedTime = time.GetLastFixedUpdateTime();
~~~~~~~~~~~~~

Fixed updates run at a consistent rate (typically 60 times per second) regardless of frame rate. Components that need consistent timing should use FixedUpdate() instead of Update():

~~~~~~~~~~~~~{.cpp}
class PhysicsComponent : public Component
{
    void FixedUpdate() override
    {
        // This runs at a fixed rate (e.g., 60 Hz)
        // Perfect for physics and other time-sensitive calculations
        SceneTime& time = SceneObject()->GetScene()->GetTime();
        float fixedDelta = time.GetFixedFrameDelta();

        ApplyPhysics(fixedDelta);
    }
};
~~~~~~~~~~~~~

# Timer intervals

For measuring time intervals, you can use the @b3d::Timer class which provides a simple interface for high-precision timing:

~~~~~~~~~~~~~{.cpp}
Timer timer; // Starts counting immediately

u64 counter = 0;
for (i32 i = 0; i < 1000000; i++)
    counter += i % 10;

u64 microseconds = timer.GetMicroseconds();
u64 milliseconds = timer.GetMilliseconds();

float secondsElapsed = (float)(microseconds * Time::kMicrosecToSec);
B3D_LOG(Info, LogGeneric, "Operation took {0} seconds.", secondsElapsed);
~~~~~~~~~~~~~

Timer starts counting as soon as it's constructed. You can use:
- @b3d::Timer::GetMicroseconds() - Returns elapsed time in microseconds
- @b3d::Timer::GetMilliseconds() - Returns elapsed time in milliseconds
- @b3d::Timer::Reset() - Resets the timer to zero

Optionally you can reset the timer by calling @b3d::Timer::Reset(). This will set the time elapsed to 0, and any elapsed time will be reported from the last reset call:

~~~~~~~~~~~~~{.cpp}
Timer timer;

// Do some work...
PerformTask1();

u64 task1Time = timer.GetMicroseconds();
timer.Reset(); // Reset for next measurement

// Do more work...
PerformTask2();

u64 task2Time = timer.GetMicroseconds();

B3D_LOG(Info, LogGeneric, "Task 1: {0}μs, Task 2: {1}μs", task1Time, task2Time);
~~~~~~~~~~~~~