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

SPtr<render::GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(createInformation);
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

SPtr<render::GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(createInformation);
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
SPtr<render::GpuBuffer> buffer = gpuDevice->CreateGpuBuffer(createInformation);
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
SPtr<GpuBuffer> buffer = ...;

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
SPtr<render::GpuBuffer> buffer = ...;

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
SPtr<render::GpuBuffer> gpuOnlyBuffer = ...; // Created with StoreOnGPU

// GpuBufferUtility handles staging internally
GpuBufferUtility::Write(gpuOnlyBuffer, 0, dataSize, sourceData);

// Read with blocking
Vector<u8> readBack(dataSize);
GpuBufferUtility::Read(gpuOnlyBuffer, 0, dataSize, readBack.data());

// Non-blocking read via command buffer
TAsyncOp<SPtr<MemoryDataStream>> asyncRead = GpuBufferUtility::ReadAsync(gpuOnlyBuffer, 0, dataSize, commandBuffer);
// ... later, when the command buffer completes, the async op is signaled
~~~~~~~~~~~~~

The @b3d::render::GpuBufferWriteFlag flags control behavior when writing to a buffer in GPU use:
 - @b3d::render::GpuBufferWriteFlag::Normal - Default. Expects the buffer is not in GPU use.
 - @b3d::render::GpuBufferWriteFlag::Discard - Internally reallocates buffer memory so previous GPU operations are not disturbed. Anything not written is undefined.
 - @b3d::render::GpuBufferWriteFlag::NoOverwrite - Allows writing while the GPU is using the buffer. The caller is responsible for not writing to regions the GPU is operating on.

# Binding
Once created, a buffer can be bound to a GPU program through **GpuParameterSet** by calling @b3d::GpuParameterSet::SetStorageBuffer.

~~~~~~~~~~~~~{.cpp}
SPtr<render::GpuParameterSet> parameterSet = ...;
parameterSet->SetStorageBuffer("myBuffer", buffer);
~~~~~~~~~~~~~

# Load-store buffers
Same as with textures, buffers can also be used for GPU program load-store operations. You simply need to set the @b3d::GpuBufferFlag::AllowUnorderedAccessOnTheGPU flag in the create information before creating the buffer.

~~~~~~~~~~~~~{.cpp}
GpuBufferCreateInformation createInformation = GpuBufferCreateInformation::CreateStructuredStorage(sizeof(MyData), 32);
createInformation.Flags |= GpuBufferFlag::AllowUnorderedAccessOnTheGPU;

SPtr<render::GpuBuffer> buffer = commandBuffer->GetGpuDevice().CreateGpuBuffer(createInformation);
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
