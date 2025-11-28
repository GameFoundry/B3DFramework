---
title: Load-store textures
---

We discussed textures in detail previously, but we haven't yet mentioned load-store textures. These are a type of textures that can be used in a GPU program for arbitrary reads and writes. This makes them different from normal textures, which can only be used for reading or as render targets. They are particularily useful for compute programs as they are (together with load-store buffers) the only ways to output data from such programs.

They are also known as unordered-access textures, storage textures or random write textures. In HLSL these textures have a *RW* prefix, e.g. *RWTexture2D*, and in GLSL they have an *image* prefix, e.g. *image2D*.

Creation of a load-store texture is essentially the same as for normal textures, except for the addition of the @b3d::TU_LOADSTORE usage flag.

~~~~~~~~~~~~~{.cpp}
// Creates a 2D load-store texture, 128x128 with a 4-component 32-bit floating point format
TextureCreateInformation createInformation;
createInformation.Type = TEX_TYPE_2D;
createInformation.Width = 128;
createInformation.Height = 128;
createInformation.Format = PF_RGBA32F;
createInformation.Usage = TU_LOADSTORE;

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
