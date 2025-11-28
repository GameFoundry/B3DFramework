---
title: Command buffers
---

As discussed in the render thread manual, the framework uses a multi-threaded architecture where rendering operations are executed on the render thread. All rendering operations in the framework are performed through @b3d::render::GpuCommandBuffer objects. These command buffers are the primary interface for interacting with the GPU - whether you're drawing geometry, setting pipeline states, or dispatching compute shaders.

Rendering can be a very CPU heavy operation even though GPU does all the rendering - but CPU is still the one submitting all those commands. Command buffers help address this by allowing you to record rendering commands on different threads, distributing the CPU workload more efficiently. Each command buffer can be populated with commands on any thread and submitted from any thread.

# Creation
Command buffers are created via a @b3d::render::GpuCommandBufferPool. The pool is created by calling @b3d::GpuDevice::CreateGpuCommandBufferPool with @b3d::render::GpuCommandBufferPoolCreateInformation as the parameter. The pool information specifies:
 - @b3d::GpuQueueUsage - This determines which commands may be executed on command buffers created from this pool. This can be:
   - @b3d::GQT_GRAPHICS - Command buffers that support all type of operations (default).
   - @b3d::GQT_COMPUTE - Command buffers that only support compute operations.
 - Thread ID - The thread on which the pool and its command buffers are allowed to be used.

If you know a command buffer will only execute compute operations it is beneficial to create the pool using **GQT_COMPUTE**.

~~~~~~~~~~~~~{.cpp}
GpuDevice& gpuDevice = GetApplication().GetPrimaryGpuDevice();

GpuCommandBufferPoolCreateInformation poolCreateInformation =
	GpuCommandBufferPoolCreateInformation::CreateForThisThread(GQT_GRAPHICS);

SPtr<GpuCommandBufferPool> commandBufferPool = gpuDevice.CreateGpuCommandBufferPool(poolCreateInformation);
~~~~~~~~~~~~~

Once you have a pool, you can create command buffers by calling @b3d::render::GpuCommandBufferPool::Create or @b3d::render::GpuCommandBufferPool::FindOrCreate. The latter will attempt to reuse an existing command buffer from the pool if one is available.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuCommandBuffer> commandBuffer = commandBufferPool->Create(
	GpuCommandBufferCreateInformation::Create("MyCommandBuffer"));
~~~~~~~~~~~~~

Command buffers are automatically put into a recording state when created or retrieved from the pool.

# Recording commands
Once created, you can record rendering commands by calling methods on the command buffer. These include:
- @b3d::render::GpuCommandBuffer::BeginRenderPass - Begins a render pass for rendering to a render target
- @b3d::render::GpuCommandBuffer::EndRenderPass - Ends the current render pass
- @b3d::render::GpuCommandBuffer::SetGpuGraphicsPipelineState - Sets the graphics pipeline state
- @b3d::render::GpuCommandBuffer::SetGpuComputePipelineState - Sets the compute pipeline state
- @b3d::render::GpuCommandBuffer::SetGpuParameterSet - Binds a GPU parameter set
- @b3d::render::GpuCommandBuffer::SetVertexBuffers - Binds vertex buffers
- @b3d::render::GpuCommandBuffer::SetIndexBuffer - Binds an index buffer
- @b3d::render::GpuCommandBuffer::SetVertexDescription - Sets the vertex declaration
- @b3d::render::GpuCommandBuffer::SetDrawOperation - Sets the draw operation type
- @b3d::render::GpuCommandBuffer::Draw - Draws geometry using vertex buffers
- @b3d::render::GpuCommandBuffer::DrawIndexed - Draws indexed geometry
- @b3d::render::GpuCommandBuffer::DispatchCompute - Executes a compute shader
- @b3d::render::GpuCommandBuffer::ClearRenderTarget - Clears the render target
- @b3d::render::GpuCommandBuffer::SetViewport - Sets the viewport area
- @b3d::render::GpuCommandBuffer::IssueBarriers - Issues memory and execution barriers
- @b3d::render::GpuCommandBuffer::TransitionTextureLayout - Transitions a texture layout

~~~~~~~~~~~~~{.cpp}
// Begin render pass
commandBuffer->BeginRenderPass(renderTarget, 0, RT_NONE);

// Clear the render target
commandBuffer->ClearRenderTarget(FBT_COLOR | FBT_DEPTH, Color::kBlue, 1.0f, 0, 0xFF);

// Bind pipeline state and resources
commandBuffer->SetGpuGraphicsPipelineState(pipelineState);
commandBuffer->SetVertexBuffers(0, &vertexBuffer, 1);
commandBuffer->SetIndexBuffer(indexBuffer);
commandBuffer->SetVertexDescription(vertexDescription);
commandBuffer->SetDrawOperation(DOT_TRIANGLE_LIST);
commandBuffer->SetGpuParameterSet(parameterSet);

// Draw
commandBuffer->DrawIndexed(0, indexCount, 0, vertexCount, 1, 0);

// End render pass
commandBuffer->EndRenderPass();
~~~~~~~~~~~~~

# Finishing recording
Before a command buffer can be submitted, you must call @b3d::render::GpuCommandBuffer::End to finish recording and prepare the command buffer for submission.

~~~~~~~~~~~~~{.cpp}
commandBuffer->End();
~~~~~~~~~~~~~

# Submitting
Commands queued on a command buffer will only get executed after the command buffer is submitted. Submission is done by calling @b3d::GpuDevice::SubmitCommandBuffer.

~~~~~~~~~~~~~{.cpp}
GpuDevice& gpuDevice = GetApplication().GetPrimaryGpuDevice();
gpuDevice.SubmitCommandBuffer(commandBuffer);
~~~~~~~~~~~~~

You must externally synchronize access to **GpuCommandBuffer** when passing it between threads, as it is not thread safe.

# Command buffer state
Command buffers have several states that can be queried via @b3d::render::GpuCommandBuffer::GetState:
- @b3d::render::CommandBufferState::Ready - Buffer is ready to begin recording commands
- @b3d::render::CommandBufferState::Recording - Buffer is currently recording commands
- @b3d::render::CommandBufferState::Executing - Buffer has been submitted and is executing on the GPU
- @b3d::render::CommandBufferState::Done - Buffer has finished executing

~~~~~~~~~~~~~{.cpp}
CommandBufferState state = commandBuffer->GetState();

if (state == CommandBufferState::Done)
{
	B3D_LOG(Info, Rendering, "Command buffer has finished executing");
}
~~~~~~~~~~~~~