GPU Buffers			{#gpuBuffers}
===============
[TOC]

GPU buffers (also known as generic buffers) allow you to provide data to a **GpuProgram** similar as a texture. In particular they are very similar to a one-dimensional texture. They aren't constrained by size limitations like a texture, and allow each entry in the buffer to be more complex than just a primitive data type. This allows you to provide your GPU programs with complex data easily. In bs::f they are represented using the @ref bs::ct::GpuBuffer "ct::GpuBuffer" type. 

# Creation {#gpuBuffers_a}
To create a **ct::GpuBuffer** you must fill out a @ref bs::GPU_BUFFER_DESC "GPU_BUFFER_DESC" structure and call the @ref bs::ct::GpuBuffer::create "ct::GpuBuffer::create()" method. At minimum you need to provide:
 - @ref bs::GPU_BUFFER_DESC::type "GPU_BUFFER_DESC::type" - This can be @ref bs::GBT_STANDARD "GBT_STANDARD" or @ref bs::GBT_STRUCTURED "GBT_STRUCTURED". See below for explanation of each.
 - @ref bs::GPU_BUFFER_DESC::elementCount "GPU_BUFFER_DESC::elementCount" - Number of elements in the buffer.
 - @ref bs::GPU_BUFFER_DESC::format "GPU_BUFFER_DESC::format" - Format of each individual element in the buffer as @ref bs::GpuBufferFormat "GpuBufferFormat". Only relevant for buffers with type **GBT_STANDARD**.
 - @ref bs::GPU_BUFFER_DESC::elementSize "GPU_BUFFER_DESC::elementSize" - Size (in bytes) of each element in the buffer. Only relevant for buffers with type **GBT_STRUCTURED**.
 
Standard buffers contain primitive elements (of **GpuBufferFormat** format), such as floats or ints, each with up to 4 components. In HLSL these buffers are represented using **Buffer** or **RWBuffer** types. In GLSL they are represented using **samplerBuffer** or **imageBuffer** types.
 
~~~~~~~~~~~~~{.cpp}
// Creates a standard buffer with 32 elements, each a 4-component float
GPU_BUFFER_DESC desc;
desc.type = GBT_STANDARD;
desc.elementCount = 32;
desc.format = BF_32X4F;

SPtr<GpuBuffer> buffer = GpuBuffer::create(desc);
~~~~~~~~~~~~~

Structured buffers contain elements of arbitrary size and are usually used for storing structures of more complex data. In HLSL these buffers are represented using **StructuredBuffer** or **RWStructuredBuffer** types. In GLSL they are represented using the **buffer** block, also known as shared storage buffer object.
 
~~~~~~~~~~~~~{.cpp}
struct MyData
{
	float a;
	int b;
};

// Creates a structured buffer with 32 elements, each with enough size to store the MyData struct
GPU_BUFFER_DESC desc;
desc.type = GBT_STRUCTURED;
desc.elementCount = 32;
desc.elementSize = sizeof(MyData);

SPtr<GpuBuffer> buffer = GpuBuffer::create(desc);
~~~~~~~~~~~~~ 

# Reading/writing {#gpuBuffers_b}
Reading or writing to a GPU buffer uses the same approach as other types of buffers, like index or vertex buffers. Refer back to the @ref geometry manual to see how.

# Binding {#gpuBuffers_c}
Once created buffer can be bound to a GPU program through **GpuParams** by calling @ref bs::ct::GpuParams::setBuffer(GpuProgramType, const String&, const BufferType&) "ct::GpuParams::setBuffer()".

~~~~~~~~~~~~~{.cpp}
SPtr<GpuParams> params = ...;
params->setBuffer(GPT_FRAGMENT_PROGRAM, "myBuffer", buffer);
~~~~~~~~~~~~~ 

# Load-store buffers {#gpuBuffers_d}
Same as with textures, buffers can also be used for GPU program load-store operations. You simply need to set the @ref bs::GBU_LOADSTORE "GBU_LOADSTORE" flag on @ref bs::GPU_BUFFER_DESC::usage "GPU_BUFFER_DESC::usage" before creating the buffer creation.

~~~~~~~~~~~~~{.cpp}
GPU_BUFFER_DESC desc;
... // Other options
desc.usage |= GBU_LOADSTORE;

SPtr<GpuBuffer> buffer = GpuBuffer::create(desc);
~~~~~~~~~~~~~ 

After that buffer can be bound as normal, as shown above. This is different from load-store textures which have a separate set of methods for binding in **GpuParams**.