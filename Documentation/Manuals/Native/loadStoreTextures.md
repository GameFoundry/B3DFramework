Load-store textures			{#loadStoreTextures}
===============
[TOC]

We discussed textures in detail previously, but we haven't yet mentioned load-store textures. These are a type of textures that can be used in a GPU program for arbitrary reads and writes. This makes them different from normal textures, which can only be used for reading or as render targets. They are particularily useful for compute programs as they are (together with load-store buffers) the only ways to output data from such programs.

They are also known as unordered-access textures, storage textures or random write textures. In HLSL these textures have a *RW* prefix, e.g. *RWTexture2D*, and in GLSL they have an *image* prefix, e.g. *image2D*.

Creation of a load-store texture is essentially the same as for normal textures, except for the addition of the @ref bs::TU_LOADSTORE "TU_LOADSTORE" usage flag.

~~~~~~~~~~~~~{.cpp}
// Creates a 2D load-store texture, 128x128 with a 4-component 32-bit floating point format
TEXTURE_DESC desc;
desc.type = TEX_TYPE_2D;
desc.width = 128;
desc.heigth = 128;
desc.format = PF_FLOAT32_RGBA;
desc.usage = TU_LOADSTORE;

SPtr<Texture> texture = Texture::create(desc);
~~~~~~~~~~~~~ 

You can then bind a load-store texture to a GPU program by calling @ref bs::ct::GpuParams::setLoadStoreTexture(GpuProgramType, const String&, const TextureType&, const TextureSurface&) "ct::GpuParams::setLoadStoreTexture()" as was described in an earlier chapter.

~~~~~~~~~~~~~{.cpp}
SPtr<GpuParams> params = ...;
params->setLoadStoreTexture(GPT_COMPUTE_PROGRAM, "myLoadStoreTex", texture);
~~~~~~~~~~~~~ 

Load-store textures do not support sampling using sampler states, you can only read-write their pixels directly. They also do not support mip-maps, and if your texture has multiple mip-maps you must provide a @ref bs::TextureSurface "TextureSurface" struct to **ct::GpuParams::setLoadStoreTexture()** in order to specify which mip-level to bind (by default it is the first).

~~~~~~~~~~~~~{.cpp}
SPtr<GpuParams> params = ...;

TextureSurface surface;
surface.mipLevel = 5; // Bind 5th mip-level for load-store operations
params->setLoadStoreTexture(GPT_COMPUTE_PROGRAM, "myLoadStoreTex", texture);
~~~~~~~~~~~~~ 

Load-store textures can also be bound as normal textures, for read-only operations like sampling. Note that they cannot be bound for both operations at once. Also note that load-store textures are not supported for 3D textures, and have limited support (depending on the rendering backend) for multisampled surfaces.