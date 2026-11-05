---
title: Barriers
---

A barrier tells the GPU that one access of a resource must finish, and its results become visible, before another access starts. It may also change the layout of an image.

Command buffers issue no barriers of their own. A [frame graph](../Renderer/frameGraph) plans and records every barrier its passes need, so this manual only matters for work you record on a command buffer directly, such as uploads or low-level utilities. Passes of a graph must not issue barriers.

# Resting states
Every resource has a resting state, returned by @b3d::GpuBackendUtility::GetRestingState. A resource that nothing is currently working on is expected to be in that state, which keeps barriers local: any code may read a resource at rest with no barrier, and code that needs anything else starts from a known state instead of having to track what happened to the resource earlier.

The resting state is what a resource can do without any barrier:
 - Textures are sampled from every shader stage, in **GpuImageLayout::ShaderReadOnly**.
 - Buffers are readable as vertex, index and indirect arguments, and from every shader stage.
 - Readback buffers (**GpuBufferType::StagingRead**) are readable by the host, and upload buffers (**GpuBufferType::StagingWrite**) by copies.
 - The surfaces of a window can only be accessed as attachments, and rest in their attachment layouts.

New textures start at rest, and the upload and read utilities such as @b3d::render::TextureUtility::Write and @b3d::render::GpuBufferUtility::Write leave resources at rest. The rule for your own work is the same: **whoever takes a resource out of its resting state returns it there**.

Resting states depend on the queue, so pass the queue type of the command buffer.

# Full barriers
@b3d::render::GpuCommandBuffer::IssueBarriers records barriers at one point in the command buffer. You describe each side with a @b3d::render::GpuAccessState, and the command buffer derives nothing from earlier use of the resource. Barriers cannot be issued inside of a render pass.

@b3d::render::GpuAccessState holds:
 - **Stages** - The pipeline stages accessing the resource, as @b3d::render::GpuStageFlag.
 - **Access** - Whether those stages read or write, as @b3d::GpuAccessFlag.
 - **Layout** - The layout of an image, ignored for buffers. A source in **GpuImageLayout::Undefined** discards the image's contents, which can be cheaper if you are about to overwrite them.
 - **Compression** - How compression may encode the image's contents, as @b3d::render::GpuCompressionHint. Only read from the source. **Unknown** is always correct, and a more precise value can make the barrier cheaper.
 - **Queues** - Other queues that access the resource in this state, described under the queues section below.

This is how a write to a buffer looks, taking the buffer from its resting state and returning it there:

~~~~~~~~~~~~~{.cpp}
const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*buffer, commandBuffer->GetQueueType());
const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write);

commandBuffer->IssueBarriers(GpuBufferBarrier(buffer, restingState, transferWrite));
commandBuffer->CopyBufferToBuffer(stagingBuffer, buffer, 0, 0, byteCount);
commandBuffer->IssueBarriers(GpuBufferBarrier(buffer, transferWrite, restingState));
~~~~~~~~~~~~~

Textures work the same way, and can apply to a range of subresources. A copy that overwrites a whole subresource can also discard the old contents, by using **Undefined** as the layout of the source:

~~~~~~~~~~~~~{.cpp}
const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*texture, commandBuffer->GetQueueType());
const GpuAccessState transferWrite(GpuStageFlag::Transfer, GpuAccessFlag::Write, GpuImageLayout::TransferDestination);
const GpuTextureSubresourceRange range(mipLevel, 1, arrayLayer, 1, GpuBackendUtility::GetFormatAspects(texture->GetProperties().Format));

GpuAccessState discardedRestingState = restingState;
discardedRestingState.Layout = GpuImageLayout::Undefined;

commandBuffer->IssueBarriers(GpuTextureBarrier(texture, discardedRestingState, transferWrite, range));
commandBuffer->CopyBufferToTexture(stagingBuffer, texture, 0, mipLevel, arrayLayer);
commandBuffer->IssueBarriers(GpuTextureBarrier(texture, transferWrite, restingState, range));
~~~~~~~~~~~~~

@b3d::render::GpuBufferBarrier and @b3d::render::GpuTextureBarrier cover buffers and textures. @b3d::render::GpuRenderTargetBarrier covers one surface of a render target, and is the only way to transition the surfaces of a window, which cannot be accessed as textures.

To issue several barriers at once, which is cheaper than issuing them one by one, pass arrays through @b3d::render::GpuBarriers. It refers to the barriers without copying them, so they must stay valid until the call returns.

~~~~~~~~~~~~~{.cpp}
Vector<GpuBufferBarrier> barriers;
for(const TShared<GpuBuffer>& destination : destinations)
	barriers.push_back(GpuBufferBarrier(destination, GpuBackendUtility::GetRestingState(*destination, commandBuffer->GetQueueType()), transferWrite));

commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuBufferBarrier>(barriers.data(), (u32)barriers.size())));
~~~~~~~~~~~~~

## Render passes
Before @b3d::render::GpuCommandBuffer::BeginRenderPass, barriers must transition the surfaces of the render target to the layouts of the render pass. @b3d::GpuBackendUtility::GetRenderPassAttachmentLayout returns the layout, given the surface and which surfaces of the render pass are read-only.

~~~~~~~~~~~~~{.cpp}
const GpuImageLayout colorLayout = GpuBackendUtility::GetRenderPassAttachmentLayout(RT_COLOR0, RT_NONE);
const GpuAccessState colorAttachment(GpuStageFlag::ColorAttachment, GpuAccessFlag::Read | GpuAccessFlag::Write, colorLayout);
const GpuAccessState restingState = GpuBackendUtility::GetRestingState(*texture, commandBuffer->GetQueueType());

commandBuffer->IssueBarriers(GpuTextureBarrier(texture, restingState, colorAttachment));
commandBuffer->BeginRenderPass(RenderPassCreateInformation(renderTarget));

// ... draw ...

commandBuffer->EndRenderPass();
commandBuffer->IssueBarriers(GpuTextureBarrier(texture, colorAttachment, restingState));
~~~~~~~~~~~~~

# Split barriers
A split barrier records a barrier in two halves. @b3d::render::GpuCommandBuffer::ReleaseBarriers goes after the source accesses, and @b3d::render::GpuCommandBuffer::AcquireBarriers before the destination accesses. The GPU can do the work of the barrier while unrelated commands between the halves run, so this can be cheaper than a full barrier when there is work in-between.

A @b3d::render::GpuSplitBarrier created with @b3d::GpuDevice::CreateSplitBarrier connects the two halves.

~~~~~~~~~~~~~{.cpp}
const GpuAccessState transferRead(GpuStageFlag::Transfer, GpuAccessFlag::Read);
const GpuBufferBarrier barrier(buffer, transferWrite, transferRead);

const TShared<GpuSplitBarrier> split = gpuDevice->CreateSplitBarrier();

commandBuffer->CopyBufferToBuffer(stagingBuffer, buffer, 0, 0, byteCount);
commandBuffer->ReleaseBarriers(barrier, split);

// ... unrelated work, which must not access the buffer ...

commandBuffer->AcquireBarriers(barrier, split);
commandBuffer->CopyBufferToBuffer(buffer, otherBuffer, 0, 0, byteCount);
~~~~~~~~~~~~~

The rules:
 - Both halves must receive the same barriers, in the same order.
 - Work between the halves must not access the resources of the barrier, and no other barrier of those resources may be recorded between them.
 - Each split barrier is released once and acquired once, and destroying one after only one half was recorded is an error.
 - Neither half can be recorded inside of a render pass.
 - The halves may be on different command buffers, recorded in any order and on any threads, but the command buffer of the release must be submitted before the command buffer of the acquire.

Devices with @b3d::RSC_NO_DEPTH_STENCIL_SPLIT_BARRIERS must not use split barriers for depth-stencil images on one queue. This currently applies to Vulkan, and is a workaround for a false hazard reported by its synchronization validation.

# Queues
When a resource moves between queues of different types, it has to be transferred with a split barrier. Create the split barrier with the queue types of the two halves, record each half on a command buffer of that type, and make the submission of the acquire wait for the queue of the release through its sync mask.

~~~~~~~~~~~~~{.cpp}
const TShared<GpuSplitBarrier> split = gpuDevice->CreateSplitBarrier(GQT_COMPUTE, GQT_GRAPHICS);

computeCommands->ReleaseBarriers(barrier, split);
gpuContext.SubmitCommandBuffer(computeCommands, GpuQueueMask::kNone);

graphicsCommands->AcquireBarriers(barrier, split);
gpuContext.SubmitCommandBuffer(graphicsCommands, GpuQueueMask::kAll);
~~~~~~~~~~~~~

Textures that @b3d::GpuBackendUtility::IsConcurrentlyReadable accepts, and buffers, can be read by several queues at once. Those textures are either sampleable only, or created with **TextureUsageFlag::AllowConcurrentQueueReads**. Other textures rest on the graphics queue, and must be transferred before another queue can access them.

When several queues access a resource in one state, set **GpuAccessState::Queues** to the queues involved, so backends whose image layouts depend on the queue can pick a layout that all of them can use. Both barriers around a state must name the same queues.

Only resources that several queues can access at once may name other queues.

# Validation
@b3d::render::GpuBarrierValidator is a development-only shadow of resource states, which checks that barriers agree with each other and with what the command buffer does. Frame graphs use one, but a command buffer you record yourself validates only if you give it one with @b3d::render::GpuCommandBuffer::SetValidator.

~~~~~~~~~~~~~{.cpp}
#if B3D_BUILD_TYPE_DEVELOPMENT
GpuBarrierValidator validator;
commandBuffer->SetValidator(&validator);
#endif

// ... record the command buffer ...
~~~~~~~~~~~~~

Set the validator before the command buffer records anything, and keep it alive until the command buffer is reset. Command buffers that share a validator carry the resource states over from one to the next, so they must be recorded one at a time, in the order the GPU executes them.

The validator reports, among others:
 - A barrier whose source does not match the destination of the previous barrier and the accesses since.
 - An access that its last barrier does not declare, or in the wrong layout. This includes the bindings of draws and dispatches, attachments of render passes, and copies.
 - A draw or dispatch that accesses a subresource an earlier draw or dispatch wrote, with no barrier between them.
 - An access or barrier between the halves of a split barrier, and halves that don't match.

If the config variable `gpu.ValidateRestingStates` is enabled, accesses of a resource before its first barrier are checked against its resting state, and the first barrier of a texture must declare a source.
