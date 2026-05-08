---
title: GPU Buffers
---

GPU buffers allow you to provide data to a **GpuProgram** similar to a texture, but without the size limitations of textures, and with the ability to store complex data types. They are used for everything from vertex and index data to uniform parameters and arbitrary storage. The framework provides two levels of GPU buffer access:
 - @b3d::GpuBuffer - Main-thread type that maintains a CPU-side cache. Writes are synced to the render thread automatically.
 - @b3d::render::GpuBuffer - Render-thread type that represents the actual GPU-side buffer. Provides direct mapped memory access, flushing, and invalidation.

For most use cases you work with the main-thread **GpuBuffer**, which handles synchronization transparently. The render-thread variant is used inside renderer implementations and when working directly with command buffers.

# Creation
To create a **GpuBuffer** you fill out a @b3d::GpuBufferCreateInformation structure and call @b3d::GpuDevice::CreateGpuBuffer. The create information provides factory methods for the different buffer types:

| Factory | Purpose |
|---|---|
| @b3d::GpuBufferCreateInformation::CreateVertex | Vertex buffers |
| @b3d::GpuBufferCreateInformation::CreateIndex | Index buffers |
| @b3d::GpuBufferCreateInformation::CreateUniform | Uniform/constant buffers |
| @b3d::GpuBufferCreateInformation::CreateSimpleStorage | Storage buffers with primitive element format |
| @b3d::GpuBufferCreateInformation::CreateStructuredStorage | Storage buffers with arbitrary element size |
| @b3d::GpuBufferCreateInformation::CreateStagingWrite | CPU-writable staging buffers (copy source) |
| @b3d::GpuBufferCreateInformation::CreateStagingRead | CPU-readable staging buffers (copy destination) |

## Storage buffers
Simple storage buffers contain primitive elements (of **GpuBufferFormat** format), such as floats or ints, each with up to 4 components. In HLSL these buffers are represented using **Buffer** or **RWBuffer** types. In GLSL they are represented using **samplerBuffer** or **imageBuffer** types.

~~~~~~~~~~~~~{.cpp}
// Creates a simple storage buffer with 32 elements, each a 4-component float
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateSimpleStorage(BF_32X4F, 32);

TShared<render::GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(createInformation);
~~~~~~~~~~~~~

Structured storage buffers contain elements of arbitrary size and are usually used for storing structures of more complex data. In HLSL these buffers are represented using **StructuredBuffer** or **RWStructuredBuffer** types. In GLSL they are represented using the **buffer** block, also known as shared storage buffer object.

~~~~~~~~~~~~~{.cpp}
struct MyData
{
	float a;
	int b;
};

// Creates a structured storage buffer with 32 elements, each with enough size to store the MyData struct
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(MyData), 32);

TShared<render::GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(createInformation);
~~~~~~~~~~~~~

## Memory placement flags
The @b3d::GpuBufferFlag flags control where the buffer memory is stored:
 - @b3d::GpuBufferFlag::StoreOnGPU - Buffer is placed in device memory. Fast GPU access, but CPU reads/writes require staging buffers. This is the default for most buffer types.
 - @b3d::GpuBufferFlag::StoreOnCPUWithGPUAccess - Buffer is placed in CPU-visible memory accessible to the GPU. Faster CPU updates (no staging needed), but slower GPU access through the PCI Express bus. This is the default for uniform and staging buffers.

## Suballocations
Buffers can contain multiple suballocations — logically separate regions within one physical buffer. This is more efficient than creating a separate **GpuBuffer** for each entry, because suballocated buffers can be bound using dynamic offsets on the command buffer. To create a buffer with suballocations, pass a `suballocationCount` when creating a uniform buffer:

~~~~~~~~~~~~~{.cpp}
// Creates a uniform buffer with space for 64 suballocations
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateUniform(uniformSize, GpuBufferFlag::StoreOnCPUWithGPUAccess, 64);
TShared<render::GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(createInformation);
~~~~~~~~~~~~~

Each suballocation may be larger than the requested size due to GPU alignment requirements (typically 256 bytes for uniform buffers). Use @b3d::render::GpuBuffer::GetSuballocationSize to query the actual aligned size.

A @b3d::render::GpuBufferSuballocation is a lightweight handle referencing a specific suballocation within a buffer. It provides the buffer pointer, the byte offset, and the suballocation size.

# Reading and writing

## Main-thread GpuBuffer
The main-thread @b3d::GpuBuffer maintains a CPU-side cache. All reads and writes operate on this cache, and changes are automatically synced to the render proxy:
 - @b3d::GpuBuffer::Write - Copies data from CPU memory into the cache.
 - @b3d::GpuBuffer::WriteTyped - Writes data with proper padding/alignment for GPU types (e.g. pads each row of a 3x3 matrix to 16 bytes).
 - @b3d::GpuBuffer::ZeroOut - Clears a region of the cache.
 - @b3d::GpuBuffer::Read - Reads from the cache. Only reflects CPU-written data, not GPU writes.
 - @b3d::GpuBuffer::Map - Maps a region for direct pointer access. Returns a @b3d::GpuBufferMappedScope RAII wrapper that marks the data dirty on destruction, triggering a sync.

~~~~~~~~~~~~~{.cpp}
TShared<GpuBuffer> buffer = ...;

// Write data directly
MyData data[32];
// ... populate data
buffer->Write(0, sizeof(data), data);

// Or map for direct access
{
	GpuBufferMappedScope mappedScope = buffer->Map(0, sizeof(data), GpuMapOption::Write);
	memcpy(mappedScope.GetMappedMemory(), data, sizeof(data));
} // Automatically marks dirty on scope exit, triggering render proxy sync
~~~~~~~~~~~~~

## Render-thread GpuBuffer
The render-thread @b3d::render::GpuBuffer provides direct access to GPU memory. Unlike the main-thread variant, you must manage flushing and invalidation manually:
 - @b3d::render::GpuBuffer::Write - Writes data directly to mapped GPU memory. Requires the buffer to be CPU-accessible (`StoreOnCPUWithGPUAccess`).
 - @b3d::render::GpuBuffer::WriteTyped - Writes with padding/alignment for GPU types.
 - @b3d::render::GpuBuffer::ZeroOut - Clears a region of the buffer.
 - @b3d::render::GpuBuffer::Read - Reads directly from mapped GPU memory. If the GPU wrote to the buffer, you must issue execution and memory barriers, then call @b3d::render::GpuBuffer::Invalidate before reading.
 - @b3d::render::GpuBuffer::GetMappedMemory - Returns the raw persistently-mapped memory pointer, or `nullptr` if not mappable.
 - @b3d::render::GpuBuffer::Flush - Makes CPU writes visible to the GPU. Only needed for non-coherent memory.
 - @b3d::render::GpuBuffer::Invalidate - Makes GPU writes visible to the CPU. Only needed for non-coherent memory.
 - @b3d::render::GpuBuffer::Map - Maps a region and returns a @b3d::render::GpuBufferMappedScope that automatically invalidates on read mappings and flushes on write mappings when the scope exits.

~~~~~~~~~~~~~{.cpp}
TShared<render::GpuBuffer> buffer = ...;

// Map, write, and auto-flush
{
	render::GpuBufferMappedScope mappedScope = buffer->Map(0, dataSize, GpuMapOption::Write);
	memcpy(mappedScope.GetMappedMemory(), data, dataSize);
} // Automatically flushes on scope exit

// You can also map a suballocation directly
render::GpuBufferSuballocation suballocation = ...;
{
	render::GpuBufferMappedScope mappedScope = suballocation.Map(GpuMapOption::Write);
	memcpy(mappedScope.GetMappedMemory(), data, suballocation.GetSize());
}
~~~~~~~~~~~~~

# GpuBufferUtility
@b3d::render::GpuBufferUtility provides high-level render-thread operations that handle staging buffers internally. This is the preferred way to write to GPU-only buffers from the render thread, as it transparently creates staging buffers and copy commands when needed:
 - @b3d::render::GpuBufferUtility::Write - Writes data into a buffer. If the buffer is not CPU-writable or is currently used by the GPU, it internally creates a staging buffer and issues a copy command via the provided command buffer (or an internal transfer buffer if none is provided).
 - @b3d::render::GpuBufferUtility::Read - Reads data from a buffer, staging if needed. Blocks if the buffer is in GPU use.
 - @b3d::render::GpuBufferUtility::ReadAsync - Non-blocking read via a command buffer. Returns a @b3d::TAsyncOp that is signaled when the data is ready.
 - @b3d::render::GpuBufferUtility::CreateStaging - Creates a staging buffer matching the size of a given buffer.

~~~~~~~~~~~~~{.cpp}
TShared<render::GpuBuffer> gpuOnlyBuffer = ...; // Created with StoreOnGPU

// GpuBufferUtility handles staging internally
GpuBufferUtility::Write(gpuOnlyBuffer, 0, dataSize, sourceData);

// Read with blocking
Vector<u8> readBack(dataSize);
GpuBufferUtility::Read(gpuOnlyBuffer, 0, dataSize, readBack.data());

// Non-blocking read via command buffer
TAsyncOp<TShared<MemoryDataStream>> asyncRead = GpuBufferUtility::ReadAsync(gpuOnlyBuffer, 0, dataSize, commandBuffer);
// ... later, when the command buffer completes, the async op is signaled
~~~~~~~~~~~~~

The @b3d::render::GpuBufferWriteFlag flags control behavior when writing to a buffer in GPU use:
 - @b3d::render::GpuBufferWriteFlag::Normal - Default. Expects the buffer is not in GPU use.
 - @b3d::render::GpuBufferWriteFlag::Discard - Internally reallocates buffer memory so previous GPU operations are not disturbed. Anything not written is undefined.
 - @b3d::render::GpuBufferWriteFlag::NoOverwrite - Allows writing while the GPU is using the buffer. The caller is responsible for not writing to regions the GPU is operating on.

# Binding
Once created, a buffer can be bound to a GPU program through **GpuParameterSet** by calling @b3d::GpuParameterSet::SetStorageBuffer.

~~~~~~~~~~~~~{.cpp}
TShared<render::GpuParameterSet> parameterSet = ...;
parameterSet->SetStorageBuffer("myBuffer", buffer);
~~~~~~~~~~~~~

# Load-store buffers
Same as with textures, buffers can also be used for GPU program load-store operations. You simply need to set the @b3d::GpuBufferFlag::AllowUnorderedAccessOnTheGPU flag in the create information before creating the buffer.

~~~~~~~~~~~~~{.cpp}
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(MyData), 32);
createInformation.Flags |= GpuBufferFlag::AllowUnorderedAccessOnTheGPU;

TShared<render::GpuBuffer> buffer = commandBuffer->GetGpuDevice().CreateGpuBuffer(createInformation);
~~~~~~~~~~~~~

After that the buffer can be bound as normal, as shown above. This is different from load-store textures which have a separate set of methods for binding in **GpuParameterSet**.

# Buffer pools
When you need many small buffer allocations of the same type, creating a separate **GpuBuffer** for each is wasteful. Buffer pools suballocate from larger backing buffers, reducing GPU resource overhead. Two pool types are provided:

## TransientGpuBufferPool
@b3d::render::TransientGpuBufferPool is a per-frame allocator where suballocations are automatically recycled after all in-flight frames complete (typically 3 frames). No manual release is needed — just call @b3d::render::TransientGpuBufferPool::AdvanceFrame once per frame after submitting all command buffers.

~~~~~~~~~~~~~{.cpp}
TransientGpuBufferPool stagingPool;
stagingPool.Initialize(gpuDevice, GpuBufferCreateInformation::CreateUniform(bufferSize), 256);

// Each frame:
render::GpuBufferSuballocation suballocation = stagingPool.Allocate();
{
	render::GpuBufferMappedScope mappedScope = suballocation.Map(GpuMapOption::Write);
	memcpy(mappedScope.GetMappedMemory(), data, suballocation.GetSize());
}

// At the end of the frame, after submitting command buffers:
stagingPool.AdvanceFrame(); // Recycles allocations from 3 frames ago
~~~~~~~~~~~~~

## GpuBufferPool
@b3d::render::GpuBufferPool is a persistent allocator where allocations must be explicitly released. It supports two lifetime modes:
 - Manual: call @b3d::render::GpuBufferPool::Allocate to get a @b3d::render::GpuBufferSuballocation and @b3d::render::GpuBufferPool::Release when done.
 - Tracked: call @b3d::render::GpuBufferPool::AllocateTracked to get a @b3d::render::TrackedGpuBufferSuballocation that automatically releases when destroyed (RAII).

~~~~~~~~~~~~~{.cpp}
GpuBufferPool uniformPool;
uniformPool.Initialize(gpuDevice, GpuBufferCreateInformation::CreateUniform(bufferSize), 1024);

// Manual lifetime
render::GpuBufferSuballocation suballocation = uniformPool.Allocate();
// ... use suballocation ...
uniformPool.Release(suballocation);

// Or tracked lifetime (auto-releases on destruction)
UPtr<TrackedGpuBufferSuballocation> tracked = uniformPool.AllocateTracked();
// ... use tracked (it inherits from GpuBufferSuballocation) ...
// Released automatically when tracked goes out of scope
~~~~~~~~~~~~~

Both pool types automatically grow by allocating new backing **GpuBuffer** objects when all existing suballocations are in use. Call @b3d::render::GpuBufferPool::Destroy or @b3d::render::TransientGpuBufferPool::Destroy to release all GPU resources when shutting down.

# GPU memory allocators
Backend buffer and texture creation is layered on top of a backend-private GPU memory allocator. The framework ships @b3d::TGpuTlsfAllocator — a Two-Level Segregated Fit allocator with O(1) bitmap-driven bucket lookup, full coalescing on free, leading-padding alignment splits, multi-heap growth, and (optionally) buffer-image granularity tracking. Backends parameterise the template with their own heap-backend trait; the in-tree backend is **VulkanHeapBackend**.

Each successful **TryAllocate** writes the result into a @b3d::TGpuResourceLocation — a POD that holds the heap handle, byte offset, size, and two strategy-private slot identity fields. Consumers own their location; the allocator only writes to it during initial allocation, and supplies a fresh replacement location through **MoveAllocation** when defragmentation moves the allocation. Pass the owning resource as the `owner` argument to **TryAllocate** (`nullptr` for an untracked allocation that won't participate in defragmentation).

## Vulkan plugin layout
The Vulkan plugin owns one @b3d::TGpuTlsfAllocator per Vulkan memory-type index. Each allocator's heaps are pinned to a single memory type via @b3d::VulkanHeapCreateInformation::MemoryTypeBits and inherit the type's property flags (so HOST_VISIBLE types are persistently mapped at heap creation, while DEVICE_LOCAL-only types are not). Allocator instances are created lazily on first use, so memory types that the application never touches cost nothing. Mapping memory-class hints to memory types happens at the @b3d::render::VulkanGpuDevice::AllocateMemory entry point, which takes an explicit `(requiredFlags, preferredFlags)` pair and a @b3d::GpuResourceKind:
 - GPU-only (textures, depth-stencil, vertex/index buffers): `requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT`, `preferredFlags = 0`.
 - CPU→GPU upload (`StoreOnCPUWithGPUAccess`): `requiredFlags = HOST_VISIBLE`, `preferredFlags = DEVICE_LOCAL | HOST_COHERENT`.
 - GPU→CPU readback (staging-read): `requiredFlags = HOST_VISIBLE`, `preferredFlags = HOST_COHERENT | HOST_CACHED`.
 - Linearly-tiled images: `requiredFlags = HOST_VISIBLE`, `preferredFlags = HOST_COHERENT`.

The Vulkan plugin runs defragmentation per frame inside @b3d::render::VulkanGpuDevice::EndFrame, immediately before the per-thread transfer command buffers are submitted. Soft budgets cap the work — defaults are 8 MB / 8 allocations per pass — and the integration is gated by an internal @c mDefragEnabled toggle (currently default-off until the descriptor-set / framebuffer invalidation paths land). The lifetime base @b3d::IGpuResource is in place — both @b3d::render::VulkanBuffer and @b3d::render::VulkanImage inherit it through @b3d::render::VulkanResource — and the recreate-and-swap pattern in @b3d::render::VulkanBuffer::MoveAllocation / @b3d::render::VulkanImage::MoveAllocation handles the GPU copy and old-handle deferred-destroy.

## Deferral modes
Each @b3d::TGpuTlsfAllocator instance is configured with one of two deferral modes that govern how @b3d::TGpuTlsfAllocator::Free releases the slot and how @b3d::TGpuTlsfAllocator::Defrag retires source slots after a move.

@b3d::FreeDeferralMode::FrameTracker (default, used by backends without per-resource use-count tracking — Metal, D3D12, Null today, plus the future linear / transient allocator). @b3d::TGpuTlsfAllocator::Free queues the allocation in a FIFO retire queue keyed on the current frame index, and @b3d::TGpuTlsfAllocator::Flush drains entries whose frame is no longer in flight (i.e. the current frame index has advanced by @c kMaximumFramesInFlight beyond the retired entry's frame). Defragmentation retires the source slot the same way. Backends configured for @b3d::FreeDeferralMode::FrameTracker are expected to call @b3d::TGpuTlsfAllocator::Flush once per frame after @b3d::render::GpuDevice::EndFrame to drain entries whose retire frame is no longer in flight.

@b3d::FreeDeferralMode::ResourceLifecycle (used by the Vulkan plugin, where @b3d::IGpuResource::Notify* events are fully wired). @b3d::TGpuTlsfAllocator::Free routes straight to @b3d::TGpuTlsfAllocator::FreeImmediate — the caller has already gated GPU completion through the resource's bound-count gate (the wrapper's destructor only fires once @c mUsedCount reaches zero, by which point every CB referencing it has retired). Defragmentation does not retire the source slot; the consumer's @b3d::IGpuResource::Destroy lifecycle frees it via @b3d::TGpuTlsfAllocator::FreeImmediate when the wrapper's use-count proves GPU completion. Strictly tighter than frame-based deferral — waits for the specific CBs that touched the resource, not "every CB outstanding at retire-time".

The two modes also constrain what @b3d::IGpuResource::MoveAllocation may return, see Defragmentation below.

## Frame tracking
Allocators key their deferred-free queue on a device-wide frame index exposed through the @b3d::IGpuFrameTracker interface implemented by @b3d::render::GpuDevice. @b3d::IGpuFrameTracker::GetCurrentFrameIndex reports the frame currently being recorded; @b3d::IGpuFrameTracker::IsFrameComplete reports whether a given frame is no longer in flight on any queue. The frame index ticks once per @b3d::render::GpuDevice::EndFrame after the per-backend "previous frame complete" wait returns, so observers see the new value only after the GPU has caught up. Frames are natural cross-queue sync points — once @c kMaximumFramesInFlight ticks pass since a frame, the GPU has drained every submission tagged with that frame's index regardless of which queue it ran on.

## Defragmentation
@b3d::TGpuTlsfAllocator::Defrag compacts live allocations to free up empty heaps and reduce fragmentation. The pass walks heaps high-index → low-index and within each heap walks the per-heap physical chain offset-high → offset-low. Eligible allocations are moved into either a lower-offset slot in the same heap (within-heap compaction) or a slot in a lower-index heap (multi-heap drain). Source-slot release follows the configured @b3d::FreeDeferralMode (see above): under @b3d::FreeDeferralMode::FrameTracker the allocator retires the source against the current frame index and drains it from the deferred-free queue once the frame is complete; under @b3d::FreeDeferralMode::ResourceLifecycle the allocator does not track the source — the consumer's destructor frees it via @b3d::TGpuTlsfAllocator::FreeImmediate.

For an allocation to participate, the consumer must:
 - Pass a non-null @b3d::IGpuResource pointer for the resource as the owner argument to @b3d::TGpuAllocator::TryAllocate. Allocations with a null owner are untracked; @b3d::TGpuTlsfAllocator::Defrag skips them.
 - Override @b3d::IGpuResource::MoveAllocation on the resource. The override receives the @b3d::render::GpuCommandBuffer the move's GPU copy must be recorded into and a fresh destination location passed by reference as the non-templated @b3d::GpuResourceLocation (downcast to the backend-specialised @b3d::TGpuResourceLocation to access the typed heap handle and slot identity). The override records the source→destination GPU copy on the supplied command buffer, replaces the IGpuResource's @b3d::TGpuResourceLocation with the supplied new location, recreates the placed backend object at the new memory range, and returns the @b3d::IGpuResource pointer that should own the destination from now on. The source heap / offset / size are read from the consumer's still-intact location at the moment @b3d::IGpuResource::MoveAllocation is called; the allocator never touches the consumer's location during defrag.

The deferral mode further constrains the override:
 - @b3d::FreeDeferralMode::FrameTracker requires the override to return @c this. Wrapper-swap is forbidden — the new wrapper would @b3d::IGpuResource::Destroy the old, whose destructor would free the source slot while the allocator is also retiring it. The allocator asserts on the returned pointer.
 - @b3d::FreeDeferralMode::ResourceLifecycle accepts either @c this (in-place patch) or a freshly-constructed @b3d::IGpuResource (wrapper swap). For wrapper swap, the old wrapper is @b3d::IGpuResource::Destroy-ed inside the override; its destructor calls into the backend's free path to release the source slot once the GPU is done.

Every tracked allocation (non-null owner) is a candidate regardless of in-flight or bound state. Correctness depends on submission ordering, not on filtering: the caller must submit the @b3d::render::GpuCommandBuffer passed to @b3d::TGpuTlsfAllocator::Defrag next, with no intervening submissions, so that pre-recorded CBs that reference the OLD backend object run on the GPU before the move's copy. After @b3d::IGpuResource::MoveAllocation returns, all newly recorded references go to the NEW backend object via the patched location.

Per-call soft budgets in @b3d::TGpuTlsfAllocator::DefragmentationInfo cap the work done in one @b3d::TGpuTlsfAllocator::Defrag call (default 32 MB). The returned @b3d::TGpuTlsfAllocator::DefragmentationStats reports moves attempted, moves completed, bytes moved, and whether either budget aborted the walk early.

~~~~~~~~~~~~~{.cpp}
// Inside the backend's per-frame work, after recording the frame's command buffers:
TGpuTlsfAllocator<VulkanHeapBackend>::DefragmentationInfo defragInfo;
defragInfo.MaxBytesPerCall = 16ull * 1024 * 1024;

auto stats = allocator.Defrag(*defragCommandBuffer, defragInfo);
B3D_LOG(Information, LogRenderer, "Defrag: {0} moves, {1} bytes", stats.MovesCompleted, stats.BytesMoved);

// The caller must submit defragCommandBuffer next, with no intervening submissions, so that
// pre-recorded CBs referencing the OLD backend object run on the GPU before the move copy.
~~~~~~~~~~~~~
