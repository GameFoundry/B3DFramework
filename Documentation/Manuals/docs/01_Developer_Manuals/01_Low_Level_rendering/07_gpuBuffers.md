---
title: GPU Buffers
---

GPU buffers (also known as generic buffers) allow you to provide data to a **GpuProgram** similar as a texture. In particular they are very similar to a one-dimensional texture. They aren't constrained by size limitations like a texture, and allow each entry in the buffer to be more complex than just a primitive data type. This allows you to provide your GPU programs with complex data easily. In the framework they are represented using the @b3d::render::GpuBuffer type.

# Creation
To create a **render::GpuBuffer** you must fill out a @b3d::GpuBufferCreateInformation structure and call @b3d::GpuDevice::CreateGpuBuffer. At minimum you need to provide:
 - @b3d::GpuBufferType - This can be @b3d::GpuBufferType::SimpleStorage or @b3d::GpuBufferType::StructuredStorage. See below for explanation of each.
 - Element count - Number of elements in the buffer.
 - @b3d::GpuBufferFormat - Format of each individual element in the buffer. Only relevant for buffers with type **SimpleStorage**.
 - Element size - Size (in bytes) of each element in the buffer. Only relevant for buffers with type **StructuredStorage**.

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

# Reading/writing
You can read from or write to a GPU buffer using methods provided by **render::GpuBuffer**:
- @b3d::render::GpuBuffer::WriteData - Writes data from CPU memory into the buffer. If the buffer is not CPU-accessible, a staging buffer will be used internally.
- @b3d::render::GpuBuffer::ReadData - Reads data from the buffer into CPU memory. If the buffer is not CPU-accessible, a staging buffer will be used internally.
- @b3d::render::GpuBuffer::Lock / @b3d::render::GpuBuffer::Unlock - Lock a portion of the buffer for direct CPU access. Only available on buffers that are CPU-accessible.

~~~~~~~~~~~~~{.cpp}
SPtr<render::GpuBuffer> buffer = ...;

// Write data to buffer
MyData data[32];
// ... populate data
buffer->WriteData(0, sizeof(data), data, BWT_NORMAL);

// Or lock for direct access (only if buffer is CPU-accessible)
void* lockedData = buffer->Lock(0, sizeof(data), GBL_WRITE_ONLY);
memcpy(lockedData, data, sizeof(data));
buffer->Unlock();
~~~~~~~~~~~~~

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
