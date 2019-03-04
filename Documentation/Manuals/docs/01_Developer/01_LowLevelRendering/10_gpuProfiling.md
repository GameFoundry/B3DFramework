GPU profiling 							{#gpuProfiling}
===============
[TOC]

GPU operations cannot be profiled using the CPU profiler as they are executed asynchronously. This means when you call a method that executes on the GPU (such as those on **RenderAPI**, as well as GPU resource read/write operations) it will return almost immediately, meaning the timing information reported by the CPU profiler will not be representative of the time it actually took to execute the operation. 

Therefore GPU profiling is instead handled by @ref bs::ProfilerGPU "ProfilerGPU", globally accessible from @ref bs::gProfilerGPU "gProfilerGPU()". It allows you to track execution times of GPU operations, as well as other helpful information.

# Sampling {#gpuProfiling_a}

Similar to CPU profiling, you issue sampling calls using @ref bs::ProfilerGPU::beginSample "ProfilerGPU::beginSample()" and @ref bs::ProfilerGPU::endSample "ProfilerGPU::endSample()". Any GPU commands executed between these two calls will be measured.

~~~~~~~~~~~~~{.cpp}
RenderAPI& rapi = RenderAPI::instance();

// ... bind pipeline states, buffers, etc.

// Measure how long it takes the GPU to draw something
gGPUProfiler().beginSample("mySample");
rapi.drawIndexed(0, numIndices, 0, numVertices);
gGPUProfiler().endSample("mySample");
~~~~~~~~~~~~~

Each GPU sample will measure the time to took the execute the command on the GPU, but will also measure various resource usage stats. For example it will measure number of draw calls, number of vertices/primitives drawn, render state switches, and similar.

All **ProfilerGPU::beginSample()** and **ProfilerGPU::endSample()** calls need to be placed in-between @ref bs::ProfilerGPU::beginFrame "ProfilerGPU::beginFrame()" and @ref bs::ProfilerGPU::endFrame "ProfilerGPU::endFrame()". As their name implies you should only call these methods once per frame. If working with the default renderer, they will be called for you by the renderer.

# Reporting {#gpuProfiling_b}

GPU profiler will generate a single report per **ProfilerGPU::beginFrame()** / **ProfilerGPU::endFrame()** pair. Since GPU is executing asynchronously the report might not be available as soon as you call **ProfilerGPU::endFrame()**. Instead you should call @ref bs::ProfilerGPU::getNumAvailableReports() "ProfilerGPU::getNumAvailableReports()" to check if there is a report available. If there is one you can then call @ref bs::ProfilerGPU::getNextReport "ProfilerGPU::getNextReport()" to retrieve the report. Report information is contained in a @ref bs::GPUProfilerReport "GPUProfilerReport" structure.

~~~~~~~~~~~~~{.cpp}
if(ProfilerGPU::instance().getNumAvailableReports() > 0)
{
	GPUProfilerReport report = ProfilerGPU::instance().getNextReport(); 
	gDebug().logDebug("It took " + report.frameSample.timeMs + "ms to execute " + report.frameSample.numDrawCalls + " draw calls.");
}
~~~~~~~~~~~~~

Retrieving the report removes it from the profiler. If more than one report is available then the oldest report is returned first. If you don't retrieve reports for multiple frames the system will start discarding oldest reports.