---
title: Textures
---

This manual covers working with textures at the render-thread level, including load-store textures and direct texture data manipulation.

# Load-store textures

We discussed textures in detail previously, but we haven't yet mentioned load-store textures. These are a type of textures that can be used in a GPU program for arbitrary reads and writes. This makes them different from normal textures, which can only be used for reading or as render targets. They are particularily useful for compute programs as they are (together with load-store buffers) the only ways to output data from such programs.

They are also known as unordered-access textures, storage textures or random write textures. In HLSL these textures have a *RW* prefix, e.g. *RWTexture2D*, and in GLSL they have an *image* prefix, e.g. *image2D*.

Creation of a load-store texture is essentially the same as for normal textures, except for the addition of the @b3d::TextureUsageFlag::AllowUnorderedAccessOnTheGPU usage flag.

~~~~~~~~~~~~~{.cpp}
// Creates a 2D load-store texture, 128x128 with a 4-component 32-bit floating point format
TextureCreateInformation createInformation;
createInformation.Type = TEX_TYPE_2D;
createInformation.Width = 128;
createInformation.Height = 128;
createInformation.Format = PF_RGBA32F;
createInformation.Usage = TextureUsageFlag::AllowUnorderedAccessOnTheGPU;

SPtr<GpuDevice> gpuDevice = ...;
SPtr<Texture> texture = gpuDevice->CreateTexture(createInformation);
~~~~~~~~~~~~~

You can then bind a load-store texture to a GPU program by calling @b3d::render::GpuParameterSet::SetStorageTexture as was described in an earlier chapter.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuParameterSet> parameterSet = ...;
SPtr<Texture> texture = ...;

TextureSurface surface = TextureSurface::kComplete;
parameterSet->SetStorageTexture("myLoadStoreTex", texture, surface);
~~~~~~~~~~~~~

Load-store textures do not support sampling using sampler states, you can only read-write their pixels directly. They also do not support mip-maps, and if your texture has multiple mip-maps you must provide a @b3d::TextureSurface struct to **render::GpuParameterSet::SetStorageTexture()** in order to specify which mip-level to bind (by default it is the first).

~~~~~~~~~~~~~{.cpp}
SPtr<GpuParameterSet> parameterSet = ...;
SPtr<Texture> texture = ...;

TextureSurface surface;
surface.MipLevel = 5; // Bind 5th mip-level for load-store operations
parameterSet->SetStorageTexture("myLoadStoreTex", texture, surface);
~~~~~~~~~~~~~

Load-store textures can also be bound as normal textures, for read-only operations like sampling. Note that they cannot be bound for both operations at once. Also note that load-store textures are not supported for 3D textures, and have limited support (depending on the rendering backend) for multisampled surfaces.

# Reading and writing texture data

For reading and writing texture data on the render thread, use the @b3d::render::TextureUtility class which provides static helper methods.

## Writing data

To write pixel data to a texture subresource:

~~~~~~~~~~~~~{.cpp}
SPtr<render::Texture> texture = ...;
PixelData pixelData = ...;

// Write data to mip level 0, array layer 0
render::TextureUtility::Write(texture, pixelData);

// Write to specific mip level and array layer
render::TextureUtility::Write(texture, pixelData, 2, 0); // mip 2, layer 0

// Use staging buffer via a command buffer for non-mappable textures
SPtr<GpuCommandBuffer> commandBuffer = ...;
render::TextureUtility::Write(texture, pixelData, 0, 0, TextureWriteFlag::Normal, commandBuffer);
~~~~~~~~~~~~~

**TextureUtility::Write()** automatically chooses the optimal path:
- For directly mappable textures (StoreOnCPUWithGPUAccess with LINEAR tiling): Uses **Map()** + **BulkPixelConversion**
- For non-mappable textures: Uses staging buffer + **CopyBufferToTexture**

The @b3d::render::TextureWriteFlag flags control behavior when writing to a texture that is in GPU use:
 - @b3d::render::TextureWriteFlag::Normal - Default. Expects the subresource is not in GPU use. Caller must provide a command buffer if it is, otherwise the write fails.
 - @b3d::render::TextureWriteFlag::Discard - Internally allocates new memory for the subresource, leaving old memory for the GPU to finish with. Anything not written by the caller is undefined.
 - @b3d::render::TextureWriteFlag::NoOverwrite - Allows writing while the GPU is using the texture. The caller is responsible for not overlapping GPU regions and issuing appropriate barriers.

## Reading data

To read pixel data from a texture subresource:

~~~~~~~~~~~~~{.cpp}
SPtr<render::Texture> texture = ...;
PixelData destination = texture->GetProperties().AllocBuffer(0, 0);

// Blocking read - waits for GPU to finish if texture is in use
render::TextureUtility::Read(texture, destination);

// Read from specific mip level and array layer
render::TextureUtility::Read(texture, destination, 2, 0); // mip 2, layer 0
~~~~~~~~~~~~~

For non-blocking reads that integrate with your rendering pipeline:

~~~~~~~~~~~~~{.cpp}
SPtr<render::Texture> texture = ...;
SPtr<GpuCommandBuffer> commandBuffer = ...;

// Queue async read operation
TAsyncOp<SPtr<PixelData>> asyncOp = render::TextureUtility::ReadAsync(texture, *commandBuffer, 0, 0);

// ... submit command buffer and continue other work ...

// Later, check if complete and get result
if (asyncOp.HasCompleted())
{
	SPtr<PixelData> result = asyncOp.GetResult();
	// Use the pixel data
}
~~~~~~~~~~~~~

## Direct memory mapping

For textures that support direct mapping (StoreOnCPUWithGPUAccess textures with LINEAR tiling), you can use @b3d::render::Texture::Map for direct CPU access. The **Map()** method takes a mip level, array layer, and @b3d::GpuMapOptions, and returns a @b3d::render::GpuTextureMappedScope RAII wrapper. Unlike **GpuBufferMappedScope** which provides a raw `void*`, the texture mapped scope contains a @b3d::PixelData with format, dimensions, and pitch information, enabling format-aware pixel access.

~~~~~~~~~~~~~{.cpp}
SPtr<render::Texture> texture = ...; // Must be StoreOnCPUWithGPUAccess

// Map returns RAII scope that auto-flushes on destruction
{
	render::GpuTextureMappedScope scope = texture->Map(0, 0, GpuMapOption::Write);
	if (scope.IsValid())
	{
		PixelData& pixelData = scope.GetPixelData();
		// Write directly to pixelData
		pixelData.SetColorAt(0, 0, Color::kRed);
	}
} // Automatically flushes when scope exits
~~~~~~~~~~~~~

You can also access the raw persistently-mapped memory pointer via @b3d::render::Texture::GetMappedMemory, which returns `nullptr` if the texture is not mappable.

## Flushing and invalidation

For non-coherent memory, render-thread textures provide @b3d::render::Texture::Flush and @b3d::render::Texture::Invalidate to synchronize between CPU and GPU. Unlike buffer flush/invalidate which takes byte offset and size, texture flush/invalidate operates on whole subresources specified by mip level and array layer:
 - @b3d::render::Texture::Flush - Makes CPU writes visible to the GPU.
 - @b3d::render::Texture::Invalidate - Makes GPU writes visible to the CPU. Must be called after issuing execution and memory barriers, before reading GPU-written data.

These are called automatically when using **GpuTextureMappedScope** — Flush on write-mapped scope destruction, Invalidate before read-mapped scope creation.

## Staging buffers

@b3d::render::TextureUtility::CreateStagingBuffer creates a @b3d::render::GpuBuffer (not a staging texture) sized to hold the pixel data for a given mip level. This is used internally by **TextureUtility::Write** and **TextureUtility::Read**, but can also be used directly for explicit buffer-to-texture or texture-to-buffer copies:

~~~~~~~~~~~~~{.cpp}
SPtr<render::Texture> texture = ...;

// Create a CPU-writable staging buffer for mip level 0
SPtr<render::GpuBuffer> stagingBuffer = render::TextureUtility::CreateStagingBuffer(texture, 0, false);

// Write data into the staging buffer, then copy to texture
render::GpuBufferMappedScope scope = stagingBuffer->Map(GpuMapOption::Write);
memcpy(scope.GetMappedMemory(), pixelData, stagingBuffer->GetTotalSize());
scope.Unmap();
commandBuffer->CopyBufferToTexture(stagingBuffer, texture, 0, 0, 0);
~~~~~~~~~~~~~

## Buffer-texture copies

For explicit control over buffer-to-texture transfers, use the command buffer methods:

~~~~~~~~~~~~~{.cpp}
SPtr<GpuCommandBuffer> commandBuffer = ...;
SPtr<GpuBuffer> stagingBuffer = ...;
SPtr<render::Texture> texture = ...;

// Copy from buffer to texture
commandBuffer->CopyBufferToTexture(stagingBuffer, texture, 0, 0, 0); // buffer offset, face, mip

// Copy from texture to buffer
commandBuffer->CopyTextureToBuffer(texture, stagingBuffer, 0, 0, 0); // face, mip, buffer offset
~~~~~~~~~~~~~

## Clearing textures

To clear all pixels of a texture subresource to a specific color:

~~~~~~~~~~~~~{.cpp}
SPtr<render::Texture> texture = ...;

// Clear to black
render::TextureUtility::Clear(texture, Color::kBlack);

// Clear specific mip level and array layer
render::TextureUtility::Clear(texture, Color::kBlue, 2, 0);
~~~~~~~~~~~~~
