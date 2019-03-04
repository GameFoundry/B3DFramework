---
title: Measuring time
---

Being able to tell the current time, as well as being able to tell elapsed time since the last frame is important for any real-time application. Use the @bs::Time class, accessible through @bs::gTime to retrieve global information about the time in bs::f.

# Current time

Use @bs::Time::getTime() to get the current time (since application start) in seconds.

~~~~~~~~~~~~~{.cpp}
float curTime = gTime().getTime();
gDebug().logDebug("Application was started " + toString(curTime) + " seconds ago.");
~~~~~~~~~~~~~

It's important to note this value is only updated once per frame (i.e. it stays constant throughout a frame). If you need more precise time that can be used for inter-frame measurements, use @bs::Time::getTimePrecise, which returns the current time in microseconds.

~~~~~~~~~~~~~{.cpp}
UINT64 preciseTimeStart = gTime().getTimePrecise();

UINT64 counter = 0;
for(int i = 0; i < 1000000; i++)
	counter += i % 10;

UINT64 preciseTimeEnd = gTime().getTimePrecise();
UINT64 timeElapsed = preciseTimeEnd - preciseTimeStart;

float secondsElapsed = timeElapsed * Time::MICROSEC_TO_SEC;
gDebug().logDebug("Operation took " + toString(secondsElapsed) + " seconds.");
~~~~~~~~~~~~~

> @bs::Time::MICROSEC_TO_SEC is a constant to convert between microseconds and seconds.

You should use **Time::getTime()** for most gameplay purposes, while **Time::getTimePrecise()** can be used for profiling and other similar situations.

# Elapsed time
Often it is useful to know know how much has passed since the last frame. Use @bs::Time::getFrameDelta() to get the elapsed time from the previous frame.

~~~~~~~~~~~~~{.cpp}
float elapsedTime = gTime().getFrameDelta();
gDebug().logDebug("Last frame was " + toString(elapsedTime) + " seconds ago.");
~~~~~~~~~~~~~

# Frame index
Sometimes, often for debugging purposes, it is useful to know the index of the current frame. Use @bs::Time::getFrameIdx(). Each frame the index gets incremented by one.

~~~~~~~~~~~~~{.cpp}
UINT64 frameIdx = gTime().getFrameIdx();
gDebug().logDebug("This is frame #" + toString(frameIdx));
~~~~~~~~~~~~~

# Intervals
Sometimes it is useful to measure a time interval, like we did with **Time::getTimePrecise()** with the example above. You can also use the @bs::Timer class for the same purpose, but with a slightly simpler interface.

~~~~~~~~~~~~~{.cpp}
Timer timer; // Starts counting

UINT64 counter = 0;
for(int i = 0; i < 1000000; i++)
	counter += i % 10;

float secondsElapsed = timer.getMicroseconds() * Time::MICROSEC_TO_SEC;
gDebug().logDebug("Operation took " + toString(secondsElapsed) + " seconds.");
~~~~~~~~~~~~~

Timer starts counting as soon as its constructed, and you can use @bs::Timer::getMicroseconds to retrieve the time elapsed.

Optionally you can also reset the timer by calling @bs::Timer::reset. This will set the time elapsed to 0, and any elapsed time will be reported from the last reset call.