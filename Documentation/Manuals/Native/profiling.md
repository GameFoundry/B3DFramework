Profiling code								{#profiling}
===============
[TOC]

Code profiling is an important process to determine performance bottlenecks. Profiling measures code execution times, memory allocations or various other resource usage. Banshee supports CPU and GPU profiling.

# CPU profiling {#profiling_a}
CPU profiler allow you to measure execution times of code executing on the CPU. It will also track the number of memory operations. CPU profiling is handled by the @ref bs::ProfilerCPU "ProfilerCPU" module. You use that module to set up sampling points that determine which areas of code to measure, and to retrieve reports telling you the measured results.

The profiler supports two separate measuring modes, the normal mode measures time in milliseconds, while the precise mode measures time in CPU cycles. The precise mode does have drawbacks as it is inacurrate for longer code as it will not account for OS context switches and similar. Usually you will be using the normal measuring mode, and reserve the precise mode when you need to know exactly how many cycles some relatively small operation takes.

To start profiling, issue a call to @ref bs::ProfilerCPU::beginSample "ProfilerCPU::beginSample" or @ref bs::ProfilerCPU::beginSamplePrecise "ProfilerCPU::beginSamplePrecise". Both of these methods accept a string that can be used for identifying the result when the profiler report is generated. You must follow each of these with a call to @ref bs::ProfilerCPU::endSample "ProfilerCPU::endSample" or @ref bs::ProfilerCPU::endSamplePrecise "ProfilerCPU::endSamplePrecise", which take the same identifier string as their counterparts. Any code executed between these calls will be measured.

You can also nest profiling calls, by calling another `beginSample` within an existing `beginSample` call.

Once you are done sampling your code, call @ref bs::ProfilerCPU::generateReport() "ProfilerCPU::generateReport". This will return a @ref bs::CPUProfilerReport "CPUProfilerReport" which contains both normal and precise sampling data as a hierarchy of samples. You can then use this data to print it on the screen, save it to a log, or similar.

You may also use the built-in @ref bs::ProfilerOverlay component to automatically display contents of both CPU and GPU profilers every frame, which will save you from manually parsing the profiler reports. Just add it to the scene and assign it a camera to render to.

If you just want to clear profiling data without generating a report, call @ref bs::ProfilerCPU::reset "ProfilerCPU::reset()".

## Threads {#profiling_b}
The profiler is thread-safe, but if you will be profiling code on threads not managed by the engine, you must manually call @ref bs::ProfilerCPU::beginThread "ProfilerCPU::beginThread" before any sample calls, and @ref bs::ProfilerCPU::endThread "ProfilerCPU::endThread" after all sample calls.

## Overhead {#profiling_c}
Profiler code itself will introduce a certain amount of overhead which will slightly skew profiling results. The profiler attempts to estimate its error, which is reported in the returned reports. You can choose to take this into consideration if you need really precise results.

# GPU profiling {#profiling_d}
GPU profiling is handled through @ref bs::ProfilerGPU "ProfilerGPU" module, which allows you to track execution times of GPU operations, as well as various resource usages.

Similar to CPU profiling, you issue sampling calls using @ref bs::ProfilerGPU::beginSample "ProfilerGPU::beginSample" and @ref bs::ProfilerGPU::endSample "ProfilerGPU::endSample". Any GPU commands executed between these two calls will be measured.

Each GPU sample will measure the time to took the execute the command on the GPU, but will also measure various resource usage stats. For example it will measure number of draw calls, number of vertices/primitives drawn, render state switches, and similar.

All this information will be contained in a @ref bs::GPUProfilerReport "GPUProfilerReport". To retrieve the report use @ref bs::ProfilerGPU::getNumAvailableReports() "ProfilerGPU::getNumAvailableReports()" and @ref bs::ProfilerGPU::getNextReport "ProfilerGPU::getNextReport()". 

GPU profiler always generates only a single report per frame. It will queue the reports, and the above method will return the oldest report and remove it from the queue. Only a certain small number of reports are queued, and old reports will be lost if you don't retrieve them every few frames.