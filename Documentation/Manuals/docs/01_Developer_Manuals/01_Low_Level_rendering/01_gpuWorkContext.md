---
title: GPU Work Context
---

A @b3d::GpuWorkContext owns the GPU work state needed to submit work to the GPU from a single thread or fiber. It is backed by a @b3d::GpuDevice, and a single device can have multiple contexts active at once — typically one per thread that submits GPU work.

Submitting GPU work needs more than just a command buffer: you need somewhere to record transfer/upload commands, scratch memory to stage data through, a pool to allocate parameter sets from, and a way to tell when the GPU has finished so that memory can be reclaimed. **GpuWorkContext** bundles all of this for one owner. Because each thread or fiber drives its own context, none of this state is shared between threads, and the submission path needs no locking.

> A context is **not** thread safe. A single thread or fiber must own and drive a given context — never share one between threads.

# What it provides
A context exposes everything a thread needs to submit a unit of GPU work:
 - @b3d::GpuWorkContext::SubmitCommandBuffer - Submits a recorded command buffer to the GPU.
 - @b3d::GpuWorkContext::GetTransferCommandBuffer - Returns a command buffer for transfer (copy/upload) operations.
 - @b3d::GpuWorkContext::CreateScratchGpuBuffer - Allocates a short-lived, single-use buffer (compute scratch, staging) from the context's scratch allocator. See the [GPU buffers](../Low_Level_rendering/gpuBuffers) manual.
 - @b3d::GpuWorkContext::GetParameterSetPool - Returns the pool that parameter sets allocated through this context are drawn from.
 - @b3d::GpuWorkContext::GetCompletionTracker - Returns the completion tracker the context uses to know when its GPU work has finished, so the memory it used can be reclaimed.

# Obtaining a context
There are two ways to get a context, depending on whether the work runs on the render thread or on a worker thread.

## The renderer's context
The renderer owns a long-lived context that lives on the render thread and is advanced once per frame. Retrieve it with @b3d::render::Renderer::GetGpuContext. This is the context to use for all render-thread work — command submission, resource uploads, and so on:

~~~~~~~~~~~~~{.cpp}
GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();
gpuContext.SubmitCommandBuffer(commandBuffer);
~~~~~~~~~~~~~

You do not manage this context's lifetime. The renderer recycles its transfer command buffers and reclaims its scratch memory at each frame boundary, inside @b3d::render::Renderer::EndGpuFrame. It also borrows the renderer's frame completion tracker, so its memory reclamation is keyed on the frame index (see the [GPU buffers](../Low_Level_rendering/gpuBuffers) manual for how frame completion is tracked).

## A custom worker context
For one-off GPU work driven by a single thread or fiber — for example importing an asset off the render thread — create your own context with @b3d::GpuWorkContext::Create. A worker context owns its own completion tracker (a timeline fence), so it tracks and waits for its own GPU work independently of the renderer's frame loop:

~~~~~~~~~~~~~{.cpp}
TShared<GpuDevice> device = ...;

// Create a context for this thread's work
TShared<GpuWorkContext> gpuContext = GpuWorkContext::Create(*device);

// Record uploads and submit command buffers through the context
TShared<render::Texture> texture = ...;
PixelData pixelData = ...;
render::TextureUtility::Write(*gpuContext, texture, pixelData);

// When the context goes out of scope its destructor submits any pending work, blocks until the
// GPU finishes it, and reclaims the memory the context allocated.
~~~~~~~~~~~~~

When a worker context is destroyed it settles its own GPU work: it submits any pending transfers, blocks (yieldably) until the GPU drains its last submission, then reclaims the scratch memory it allocated. This makes a worker context a self-contained scope — create it, record and submit work, and let it go out of scope to flush and wait. You can also drive this explicitly with @b3d::GpuWorkContext::WaitAndReclaim.

# Submitting work
Commands recorded on a command buffer only execute once the buffer is submitted. Submit through the context that owns the work with @b3d::GpuWorkContext::SubmitCommandBuffer. Submitting first flushes any pending transfer commands recorded on the context, so uploads are visible to the work that follows:

~~~~~~~~~~~~~{.cpp}
GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();

// Any transfers recorded on the context are flushed before this command buffer runs
gpuContext.SubmitCommandBuffer(commandBuffer);
~~~~~~~~~~~~~

See the [command buffers](../Low_Level_rendering/commandBuffers) manual for how to record a command buffer.

# Transfers and uploads
Copy and upload work is recorded into a dedicated transfer command buffer, obtained with @b3d::GpuWorkContext::GetTransferCommandBuffer. The high-level @b3d::render::GpuBufferUtility and @b3d::render::TextureUtility helpers record into it for you, staging data through the context's scratch memory when needed — they take the context as their first argument (see the [GPU buffers](../Low_Level_rendering/gpuBuffers) and [load-store textures](../Low_Level_rendering/loadStoreTextures) manuals).

Recorded transfers are submitted automatically before the next command buffer you submit through the context. To force them to execute on their own — for example before reading back a resource the transfers write — call @b3d::GpuWorkContext::SubmitTransferCommandBuffers.

# Lifetime rules
A few rules follow from a context being single-owner state:
 - Use a context only on the thread or fiber that owns it.
 - Do not retain anything allocated from a context's scratch memory (scratch buffers, staging buffers) past the frame or operation that created it — that memory is reclaimed in bulk once the GPU work completes.
 - Parameter sets allocated from a context's pool must not outlive the context.
 - A worker context's destructor waits for its GPU work; the renderer's context is instead advanced and reclaimed per frame by the renderer.
