---
title: Profiling
---

Code profiling is an important process to determine performance bottlenecks. Profiling measures code execution times and memory allocations. Framework provides a built-in profiler through the @b3d::ProfilerCPU module. This module can be globally accessed through @b3d::GetProfilerCPU().

The profiler allows you to profile blocks of code and output information about how long the block took to execute, as well as information about number and amount of memory allocations.

The profiler supports two separate measuring modes, the normal mode measures time in milliseconds, while the precise mode measures time in CPU cycles. The precise mode does have drawbacks as it is inacurrate for longer code as it will not account for OS context switches and similar. Usually you will be using the normal measuring mode, and reserve the precise mode when you need to know exactly how many cycles some relatively small operation takes.

# Recording
To start profiling a block call surround it with either:
 - @b3d::ProfilerCPU::BeginSample / @b3d::ProfilerCPU::EndSample - Records timing information (in milliseconds) about the code in-between, as well as memory allocation information
 - @b3d::ProfilerCPU::BeginSamplePrecise / @b3d::ProfilerCPU::EndSamplePrecise - Records timing information (in CPU cyles) about the code in-between, as well as memory allocation information

All of the methods above expect a name as a parameter, which is arbitrary and it is used so you can later identify the profiling information.

~~~~~~~~~~~~~{.cpp}
void DoSomethingIntensive()
{
	// ...
}

GetProfilerCPU().BeginSample("myProfilingBlock");
DoSomethingIntensive();
GetProfilerCPU().EndSample("myProfilingBlock");
~~~~~~~~~~~~~

Each sample needs to have a *Begin()* and an *End()* pair. Samples can be nested between other samples.

# Reporting
Once you have placed sample points around your code, you can retrieve the profiling report by calling @b3d::ProfilerCPU::GenerateReport(). This will return a @b3d::CPUProfilerReport object, which contains a list of normal and precise samples.

Each sampling entry is represented either by @b3d::CPUProfilerBasicSamplingEntry or @b3d::CPUProfilerPreciseSamplingEntry. Sampling entries contain information about the time it took to execute the code in the sampled block of code, as well as number of memory allocations & deallocations, and total number of allocated and deallocated bytes. Each sample also contains a list of child samples (if any).

~~~~~~~~~~~~~{.cpp}
CPUProfilerReport report = GetProfilerCPU().GenerateReport();
const CPUProfilerBasicSamplingEntry& basicEntry = report.GetBasicSamplingData();
B3D_LOG(Info, LogGeneric, "Basic sampling - Calls: {0}, Avg time: {1}ms",
	basicEntry.Data.NumCalls, basicEntry.Data.AvgTimeMs);

// Process child entries
for(const CPUProfilerBasicSamplingEntry& childEntry : basicEntry.ChildEntries)
{
	B3D_LOG(Info, LogGeneric, "{0} took {1}ms", childEntry.Data.Name, childEntry.Data.TotalTimeMs);
}

const CPUProfilerPreciseSamplingEntry& preciseEntry = report.GetPreciseSamplingData();
B3D_LOG(Info, LogGeneric, "Precise sampling - Calls: {0}, Avg cycles: {1}",
	preciseEntry.Data.NumCalls, preciseEntry.Data.AvgCycles);
~~~~~~~~~~~~~

After retrieving the data, you can log it, display it on screen, or similar.

## Profiler overlay
You can easily display the profiler reports on screen by calling @b3d::Application::ShowProfilerOverlay with a camera to display the overlay on. If no camera is provided the system will use the main camera in the scene (if any). The method expects a parameter of type @b3d::ProfilerOverlayType which controls whether to display CPU or GPU profiling data.

~~~~~~~~~~~~~{.cpp}
// Display CPU sample overlay on the main camera
GetApplication().ShowProfilerOverlay(ProfilerOverlayType::CPUSamples);
~~~~~~~~~~~~~

To hide the overlay call @b3d::Application::HideProfilerOverlay.

## Threads
The profiler is thread-safe, but if you are profiling code on threads not managed by the engine, you must manually call @b3d::ProfilerCPU::BeginThread before any sample calls, and @b3d::ProfilerCPU::EndThread after all sample calls.

## Overhead
Profiler code itself will introduce a certain amount of overhead which will slightly skew profiling results. The profiler attempts to estimate this error, which is reported in the returned reports. You can choose to take this into consideration if you need really precise results.
