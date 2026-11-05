---
title: Transient resources
---

A transient resource is a texture or a buffer that only lives for part of a piece of GPU work. Most frames use many of them, such as G-buffers, intermediate post-processing textures and scratch buffers, and few of them are in use at the same time. Placing them in shared memory, with resources whose lifetimes don't overlap reusing the same bytes, uses far less memory than giving each its own.

The usual way to use transients is through @b3d::render::FrameGraph::CreateTexture and @b3d::render::FrameGraph::CreateBuffer, described in the [frame graph](../Renderer/frameGraph) manual, in which case the graph does everything on this page for you. This manual explains what happens underneath, and how to use the allocator without a graph. The examples are in the `b3d::render` namespace.

> The word "transient" is also used for two unrelated things. **AllocateTransient()** on uniform buffer definitions (see the [GPU programs](../Low_Level_rendering/gpuPrograms) manual) and @b3d::render::TransientGpuBufferPool (see the [GPU buffers](../Low_Level_rendering/gpuBuffers) manual) hand out host-writable memory that is recycled a few frames later. Those are CPU-written, per-frame data. The resources on this page are GPU-only, and share memory with other resources.

# How memory gets shared
Three objects are involved, from the highest level to the lowest:
 - @b3d::GpuTransientResourceAllocator creates and caches the resources, and places them in memory. Each @b3d::GpuWorkContext has one, returned by @b3d::GpuWorkContext::GetTransientResourceAllocator.
 - @b3d::GpuAliasingAllocator decides where in the heaps a resource goes, and only deals with memory ranges. The transient resource allocator uses one internally.
 - @b3d::IGpuTransientHeapPool owns the heaps. The device has the pools, shared by all allocators, and @b3d::GpuDevice::GetTransientHeapPool returns the one for a memory type. Memory types that cannot be aliased have no pool, and resources of those types go in persistent memory instead.

Placement happens within a scope, opened with @b3d::GpuTransientResourceAllocator::BeginScope. A frame graph opens one scope per execution. Every scope starts with all heap memory free, so scopes share memory with each other, including scopes of different allocators.

# Submissions and ordering
Memory released by one resource can only be reused by another if the GPU work of the first is ordered before the work of the second. To know this the scope needs a @b3d::GpuSubmissionTimeline, which lists the planned submissions in the order they will be submitted, along with the queues each one waits on.

~~~~~~~~~~~~~{.cpp}
GpuSubmissionTimeline timeline;
const u32 submission = timeline.AddSubmission(GpuQueueId(GQT_GRAPHICS, 0), GpuQueueMask::kNone);
~~~~~~~~~~~~~

The timeline must not change until the scope ends, and the scope's work must be submitted in timeline order.

# Allocating
@b3d::GpuTransientScope places resources. All placements and releases of a scope must happen before any of its GPU work is recorded, in an order consistent with the execution order on each queue. This means you plan a whole scope up front:

~~~~~~~~~~~~~{.cpp}
GpuTransientResourceAllocator& allocator = gpuContext.GetTransientResourceAllocator();
GpuTransientScope& scope = allocator.BeginScope(timeline);

// Allocate the first texture, and release it after its last use in the planned work
const GpuTransientTexture first = scope.AllocateTexture(textureInformation, submission);

GpuAccessScope firstAccess;
firstAccess.Add(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);

const GpuTransientLastUse firstLastUse(submission, firstAccess, 0);
scope.Release(*first.Texture, TArrayView<const GpuTransientLastUse>(&firstLastUse, 1));

// May land on the memory of the first texture, as the first texture's use comes before it
const GpuTransientTexture second = scope.AllocateTexture(textureInformation, submission);

GpuAccessScope secondAccess;
secondAccess.Add(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write);

const GpuTransientLastUse secondLastUse(submission, secondAccess, 1);
scope.Release(*second.Texture, TArrayView<const GpuTransientLastUse>(&secondLastUse, 1));

// ... record the work of the scope ...

allocator.EndScope();
~~~~~~~~~~~~~

Each @b3d::GpuTransientLastUse describes the last accesses of the resource on one queue, so a resource used on two queues reports two of them. A resource that was never used can report none. Every placed resource must be released before the scope ends.

The resources you get back are valid from their placement until their release, and their contents are undefined at their first use, even if the allocator returns a resource from an earlier scope.

A resource can be requested before you know when it is first used, which is what frame graphs do. @b3d::GpuTransientResourceAllocator::GetOrCreateTexture returns a texture with no memory, whose properties are valid and which can be referenced right away. @b3d::GpuTransientScope::PlaceTexture later gives it memory. An unplaced texture cannot be accessed on the GPU. If it turns out not to be needed, give it back with @b3d::GpuTransientResourceAllocator::Return, or take ownership of it with @b3d::GpuTransientResourceAllocator::Detach. Check @b3d::GpuTransientPlacement::Recreated after placing, because if it is true, then views and render targets created from the resource before the placement are no longer valid.

# Alias acquires and releases
The GPU does not know that two resources occupy the same bytes, so a resource taking over memory has to be synchronized with the resources that used it before. This takes two barriers (see the [barriers](../Low_Level_rendering/barriers) manual for barriers in general):
 - An **alias acquire**, with **GpuBarrierFlag::AliasAcquire**, starts the lifetime of the new resource. It orders the first use of the resource after the last uses of the earlier resources on the memory. It must be issued before any other use of the resource, and leaves the contents of the resource undefined.
 - An **alias release**, with **GpuBarrierFlag::AliasRelease**, ends the lifetime of an earlier texture on the memory.

Each allocation returns a @b3d::render::GpuAliasAcquire, which remains valid until the end of the scope and tells you what those barriers need:
 - **Source** - The last accesses of the earlier resources on the memory. The source of the alias acquire must cover them.
 - **HasUnknownSourceAccess** - Set if part of the memory wasn't used by this scope, but may have been by earlier ones. The alias acquire must then also be ordered after all earlier work on its queue.
 - **Predecessors** - The earlier resources on the memory, which can't be used again until they are acquired again.

## Example
This records the two textures from the allocation example above, one after the other. Whether the second texture landed on the memory of the first is up to the allocator, so its predecessors decide if an alias release is needed:

~~~~~~~~~~~~~{.cpp}
const GpuAccessState colorAttachment(GpuStageFlag::ColorAttachment, GpuAccessFlag::Write, GpuImageLayout::ColorAttachment);

const auto fnGetAcquireSource = [&commandBuffer](const GpuAliasAcquire& acquire)
{
	GpuAccessState source(acquire.Source.GetStages(), acquire.Source.GetAccess());

	// Earlier scopes may have used part of the memory, in ways this scope knows nothing about
	if(acquire.HasUnknownSourceAccess)
	{
		source.Stages |= GpuStageFlags(GpuStageFlag::All) & ~GpuStageFlags(GpuStageFlag::Host);
		source.Access |= GpuAccessFlag::Write;
	}

	source.Stages &= GpuBackendUtility::GetQueueStageFlags(commandBuffer->GetQueueType());
	return source;
};

// The first texture of the scope has no predecessors, so it only needs an alias acquire
commandBuffer->IssueBarriers(GpuTextureBarrier(first.Texture, fnGetAcquireSource(first.Acquire), colorAttachment,
	GpuTextureSubresourceRange::AllSubresources(), GpuBarrierFlag::AliasAcquire));

// ... render to the first texture ...

// The second texture starts its lifetime
Vector<GpuTextureBarrier> barriers;
barriers.push_back(GpuTextureBarrier(second.Texture, fnGetAcquireSource(second.Acquire), colorAttachment,
	GpuTextureSubresourceRange::AllSubresources(), GpuBarrierFlag::AliasAcquire));

// If it took over the memory of the first texture, that ends the lifetime of the first texture
const GpuAccessState releasedState(colorAttachment.Stages, GpuAccessFlag::None, GpuImageLayout::Undefined);
for(IGpuResource* predecessor : second.Acquire.Predecessors)
{
	if(predecessor == first.Texture->GetGpuResource())
	{
		barriers.push_back(GpuTextureBarrier(first.Texture, colorAttachment, releasedState,
			GpuTextureSubresourceRange::AllSubresources(), GpuBarrierFlag::AliasRelease));
	}
}

commandBuffer->IssueBarriers(GpuBarriers(TArrayView<const GpuTextureBarrier>(barriers.data(), (u32)barriers.size())));

// ... render to the second texture ...
~~~~~~~~~~~~~

An alias acquire of a texture must cover the whole texture, and its source must have **GpuImageLayout::Undefined**, which is what the source above gets by default. Its destination is the state of the resource's first use. Buffers use the same flag on a @b3d::render::GpuBufferBarrier.

An alias release has these requirements:
 - The source is the state the texture's last barrier left it in. If its subresources are in different states, issue one alias release for each.
 - The destination has **GpuImageLayout::Undefined** and no access, and its stages are those of the new resource's first use.
 - It is the last barrier of the texture, and is recorded in the same call as the alias acquire, or before it.
 - A texture's lifetime only ends once. If several new resources take over its memory, it goes with the first of their alias acquires.

**GpuAliasAcquire::Source** only covers accesses on the queue that records the alias acquire. Accesses on other queues must already be ordered through queue waits, and the allocator only reuses memory if the timeline shows that they are.

## Split barriers
If other work runs between the last use of the old resource and the first use of the new one, the same barriers can be issued as a split barrier, so the transition does not block that work. @b3d::GpuTransientTexture::LatestPredecessorPassIndex reports the latest pass among the earlier accesses, using the pass indices you gave to @b3d::GpuTransientLastUse. The release goes right after that pass, and the acquire before the first use:

~~~~~~~~~~~~~{.cpp}
const TShared<GpuSplitBarrier> split = gpuDevice->CreateSplitBarrier();

// ... render to the first texture ...

commandBuffer->ReleaseBarriers(barriers, split);

// ... unrelated work, which must not access either texture ...

commandBuffer->AcquireBarriers(barriers, split);

// ... render to the second texture ...
~~~~~~~~~~~~~

## Validation
A @b3d::render::GpuBarrierValidator doesn't know about the allocator, so you tell it about each alias acquire before recording:

~~~~~~~~~~~~~{.cpp}
#if B3D_BUILD_TYPE_DEVELOPMENT
GpuBarrierValidator validator;
commandBuffer->SetValidator(&validator);

validator.DeclareAliasAcquire(*first.Texture, first.Acquire);
validator.DeclareAliasAcquire(*second.Texture, second.Acquire);
#endif
~~~~~~~~~~~~~

It then reports a resource used before its alias acquire, an alias acquire whose source does not cover **GpuAliasAcquire::Source**, and an earlier resource that is used after a new one took over its memory. Barriers issued in one call are not ordered against each other, so it also reports an earlier resource's barrier in the same call as the alias acquire, unless it is an alias release.

# Scopes and threads
## One open scope per allocator
While a scope is open the allocator holds the scope's timeline and the memory it has placed, and it holds them for that one scope only. Calling @b3d::GpuTransientResourceAllocator::BeginScope while a scope of the same allocator is open triggers an assert, so scopes of one allocator run one after the other and can't be nested:

~~~~~~~~~~~~~{.cpp}
GpuTransientResourceAllocator& allocator = gpuContext.GetTransientResourceAllocator();

GpuTransientScope& scope = allocator.BeginScope(timeline);

// ... place and release resources, then record the work ...

allocator.EndScope();

// May get the same resources back, with undefined contents
GpuTransientScope& nextScope = allocator.BeginScope(nextTimeline);

// ... place and release resources, then record the work ...

allocator.EndScope();
~~~~~~~~~~~~~

A frame graph keeps the scope of its work context's allocator open from @b3d::render::FrameGraph::Compile until the end of @b3d::render::FrameGraph::Execute. A graph must therefore be executed before another graph of the same work context is compiled, and a pass function can't compile a graph of the work context that is executing it. Creating resources and adding passes doesn't need the scope, so several graphs of one work context can be built at the same time.

Work that needs a scope while another one is open needs another allocator, which means another @b3d::GpuWorkContext.

## Threads
An allocator and its scope have no locks. They must only be used from the thread or fiber that owns their work context, and this includes calls that only read, such as @b3d::GpuTransientResourceAllocator::GetStatistics.

This limits placement, not recording. All placements and releases of a scope happen before its work is recorded, so other threads can record the scope's command buffers using the resources placed earlier, as graphs created with **FrameGraphFlag::RecordInParallel** do. Those threads must finish before the owning thread calls @b3d::GpuTransientResourceAllocator::EndScope.

To place resources on several threads, give each thread a work context of its own, and with it an allocator:

~~~~~~~~~~~~~{.cpp}
// On each thread
const TShared<GpuWorkContext> threadContext = GpuWorkContext::Create(*gpuDevice);
GpuTransientResourceAllocator& threadAllocator = threadContext->GetTransientResourceAllocator();

GpuTransientScope& scope = threadAllocator.BeginScope(timeline);
~~~~~~~~~~~~~

The heap pools are thread safe, so these allocators can place memory at the same time, on the same pools, with no locking on your part.

## Ordering scopes that share memory
Allocators know nothing about each other's placements. Each scope sees all heap memory as free even while resources of another scope are in use, so resources of two scopes will often occupy the same bytes. Only the order of the GPU work makes this safe, and that is your responsibility:
 - All submissions of a scope must reach the queues together, with no submission of another scope between them. With scopes A and B, the order A1, A2, B1, B2 is correct, because the first use of B's memory waits for all earlier work on its queue. The order A1, B1, A2, B2 is not, because B1 can overwrite memory that A2 still uses.
 - A scope that uses more than one queue must also order the queues. Its first submission on any other queue must wait for the graphics queue, and its last graphics submission must wait for the other queues. An alias acquire only orders work on its own queue, and this way the work of one scope still finishes before the work of the next one starts.

So threads can place and record at the same time, but must take turns submitting, for example by submitting each finished scope as a whole under a lock of your own. Frame graphs follow these rules on their own, as long as only one graph executes at a time, and development builds report overlapping executions as an error. Nothing reports a violation for scopes you manage yourself.

# Caching
Resources are cached by their description and memory location. A scope that places the same resources in the same order as an earlier scope gets the same resources back, without recreating native resources. Since graphs usually repeat from frame to frame, placement is normally just a lookup.

Cached resources that went unused for a number of frames are released, and if the cache grows too large the least recently used are evicted. Heaps without references are released in the same way. @b3d::GpuTransientResourceAllocator::GetStatistics reports the memory use of the last scope, and how many placements hit the cache. The difference between **GpuTransientScopeStatistics::RequestedBytes** and **GpuTransientScopeStatistics::PeakUsed** is the memory aliasing saved.

# Rules
 - All of a scope's work must be recorded before @b3d::GpuTransientResourceAllocator::EndScope, and submitted within the frame the scope ended in.
 - Two scopes must not record into the same command buffer.
 - Transient resources can only be accessed by the GPU.

# Config variables
 - `gpu.TransientResources` - If disabled, transient resources are created in persistent memory, and never share memory with other resources.
 - `gpu.TransientAliasing` - If disabled, no two transient resources of one scope share memory. Scopes still share memory with each other.
 - `gpu.TransientAliasAcrossQueues` - If disabled, memory last used in the scope on another queue than the queue of the new resource's first use is not reused.
 - `gpu.TransientHeapMinimumSize` - Size of a new transient heap in megabytes, unless the resource that requires the heap is larger. 128 by default.
 - `gpu.TransientIdleFrames` - Number of frames a heap without references, or a cached resource, may go unused before it is released. 30 by default.
 - `gpu.TransientMaxCachedTextures` and `gpu.TransientMaxCachedBuffers` - Maximum number of textures and buffers each allocator caches. 256 by default.
