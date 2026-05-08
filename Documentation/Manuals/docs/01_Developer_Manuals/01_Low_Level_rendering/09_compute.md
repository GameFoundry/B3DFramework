---
title: Compute
---

Compute GPU programs are not meant to be used for drawing/rendering, but instead for arbitrary computations. In order to execute a compute program you must bind a **GpuComputePipelineState** to a **GpuCommandBuffer**. After it is bound you must call @b3d::render::GpuCommandBuffer::DispatchCompute().

Compute GPU programs are executed in one or multiple thread groups. Each thread group has one or multiple threads, as defined in the GPU program code itself. Thread groups and threads can be organized in one, two or three dimensions, depending on what is most relevant to the data being processed. **GpuCommandBuffer::DispatchCompute()** expects the number of thread-groups to launch as parameters.

~~~~~~~~~~~~~{.cpp}
// Get the GPU device from the command buffer
TShared<GpuDevice> device = commandBuffer->GetGpuDevice();

// Create a compute pipeline state with your compute program
GpuComputePipelineStateCreateInformation pipelineCreateInfo;
pipelineCreateInfo.Program = myComputeProgram;
TShared<GpuComputePipelineState> computePipelineState = device->CreateGpuComputePipelineState(pipelineCreateInfo);

// Create GPU parameter set for binding resources (set 0)
TShared<GpuParameterSet> parameterSet = device->CreateGpuParameterSet(computePipelineState->GetParameterLayout()->GetSet(0), 0);

// Set parameters (buffers, textures, etc.)
parameterSet->SetStorageBuffer("inputBuffer", myInputBuffer);
parameterSet->SetStorageBuffer("outputBuffer", myOutputBuffer);

// Bind the pipeline state and parameters
commandBuffer->SetGpuComputePipelineState(computePipelineState);
commandBuffer->SetGpuParameterSet(parameterSet);

// Execute a GPU program with 32x32 thread-groups
commandBuffer->DispatchCompute(32, 32, 1);

// Log completion
B3D_LOG(Info, LogRenderBackend, "Compute dispatch completed with 32x32 thread groups");
~~~~~~~~~~~~~

Note that compute operations are recorded in a **GpuCommandBuffer** which must then be submitted to a queue for execution. You can set up parameters using **GpuParameterSet** and bind them before dispatching the compute shader.
