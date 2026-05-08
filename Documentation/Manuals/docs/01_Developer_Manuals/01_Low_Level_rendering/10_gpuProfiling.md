---
title: GPU profiling
---

GPU operations cannot be profiled using the CPU profiler as they are executed asynchronously. This means when you call a method that executes on the GPU (such as those on **GpuCommandBuffer**, as well as GPU resource read/write operations) it will return almost immediately, meaning the timing information reported by the CPU profiler will not be representative of the time it actually took to execute the operation.

Therefore GPU profiling is instead handled by @b3d::GpuProfiler, globally accessible from @b3d::GetGpuProfiler. It allows you to track execution times of GPU operations, as well as other helpful information.

# Setting up profiling

GPU profiling is done on a per-command-buffer basis. Before you can record samples, you need to enable profiling on a @b3d::render::GpuCommandBuffer by calling @b3d::render::GpuCommandBuffer::BeginProfiling. This returns a @b3d::GpuCommandBufferProfiler that you can use to record samples. When you're done recording commands, call @b3d::render::GpuCommandBuffer::EndProfiling.

~~~~~~~~~~~~~{.cpp}
TShared<GpuCommandBuffer> commandBuffer = commandBufferPool->FindOrCreate(GpuCommandBufferCreateInformation::Create("MyCommandBuffer"));

// Enable profiling on the command buffer
TShared<GpuCommandBufferProfiler> commandBufferProfiler = commandBuffer->BeginProfiling("MyProfilingScope");

// ... record commands and samples ...

// Finish profiling
commandBuffer->EndProfiling();
~~~~~~~~~~~~~

# Sampling

Once profiling is enabled, you issue sampling calls using @b3d::GpuCommandBufferProfiler::BeginSample and @b3d::GpuCommandBufferProfiler::EndSample. Any GPU commands executed between these two calls will be measured.

~~~~~~~~~~~~~{.cpp}
TShared<GpuCommandBuffer> commandBuffer = commandBufferPool->FindOrCreate(GpuCommandBufferCreateInformation::Create("MyCommandBuffer"));

// Enable profiling
TShared<GpuCommandBufferProfiler> commandBufferProfiler = commandBuffer->BeginProfiling("MyProfilingScope");

// ... bind pipeline states, buffers, etc.

// Measure how long it takes the GPU to draw something
commandBufferProfiler->BeginSample(*commandBuffer, "DrawSample");
commandBuffer->DrawIndexed(0, indexCount, 0, vertexCount);
commandBufferProfiler->EndSample(*commandBuffer);

// Finish profiling
commandBuffer->EndProfiling();
~~~~~~~~~~~~~

Each GPU sample will measure the time it took to execute the command on the GPU, but will also measure various resource usage stats. For example it will measure number of draw calls, number of vertices/primitives drawn, render state switches, and similar.

All **GpuCommandBufferProfiler::BeginSample()** and **GpuCommandBufferProfiler::EndSample()** calls must be placed in-between the **GpuCommandBuffer::BeginProfiling()** and **GpuCommandBuffer::EndProfiling()** calls. If a command buffer is currently within a render pass, **EndSample()** must also be issued within a render pass. If a command buffer is currently outside of a render pass, **EndSample()** must be issued outside of a render pass.

# Reporting

After you call **GpuCommandBuffer::EndProfiling()**, the profiler automatically submits the results for resolution. The profiling results will be generated once the command buffer has finished executing on the GPU. Since GPU execution is asynchronous, the results might not be available immediately. You can retrieve the results by calling @b3d::GpuProfiler::GetResults with the profiling scope name you provided to **BeginProfiling()**. If results are available, this method returns a @b3d::GpuProfilerResults structure. If no results are available yet, it returns an empty optional.

~~~~~~~~~~~~~{.cpp}
TOptional<GpuProfilerResults> results = GetGpuProfiler().GetResults("MyProfilingScope");

if(results.HasValue())
{
	for(const GpuProfilerSample& sample : results.Value().Samples)
	{
		B3D_LOG(Verbose, LogUncategorized, "Sample {0} took {1}ms to execute {2} draw calls.", sample.Name, sample.TimeMs, sample.DrawCallCount);
	}
}
~~~~~~~~~~~~~

Retrieving the results removes them from the profiler. Each **GetResults()** call returns the latest available results for the specified profiling scope. If you don't retrieve results for multiple frames, the system will start discarding the oldest results.
