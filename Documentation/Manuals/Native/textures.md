Textures									{#textures}
===============
[TOC]

Textures in Banshee are represented with the @ref BansheeEngine::Texture "Texture" and @ref BansheeEngine::TextureCore "TextureCore" classes. Both of these provide almost equivalent functionality, but the former is for use on the simulation thread, and the latter is for use on the core thread. If you are confused by the dual nature of the objects, read the [core thread](@ref coreThread) manual. 

We're going to focus on the simulation thread implementation in this manual, and then note the differences in the core thread version at the end.

# Creating a texture {#textures_a}
To create a texture call @ref BansheeEngine::Texture::create "Texture::create" or one if its overloads. At minimum you need to provide a @ref BansheeEngine::TextureType "texture type", dimensions and @ref BansheeEngine::PixelFormat "pixel format". The dimensions range from one to three dimensional depending on the texture type.

Optionally you can also provide the number of mipmaps, number of samples, usage flags and a gamma correction flag:
 - A texture with mip-maps will contain a set of scaled down versions of itself that are used by the GPU for special filtering. 
 - A texture with more than one sample can be used only for rendering (see the [render targets](@ref renderTargets) manual), and is useful for antialiasing.
 - @ref BansheeEngine::TextureUsage "Usage flags" specify how is the texture allowed to be used.
 - Gamma correction flag specifies if the data in the texture has been gamma corrected. If enabled the GPU will transform the texture data back to linear space when it is accessing it. If the texture is already in linear space, or you do not need it to be in linear space you can leave this off.
 
For example:
~~~~~~~~~~~~~{.cpp}
// Creates a 2D texture, 128x128 with an 8-bit RGBA format
HTexture texture = Texture::create(TEX_TYPE_2D, 128, 128, 0, PF_R8G8B8A8);
~~~~~~~~~~~~~ 

You can also create a non-empty texture by creating it with a populated @ref BansheeEngine::PixelData "PixelData" object. More about @ref BansheeEngine::PixelData "PixelData" later.
 
# Accessing properties {#textures_b} 
You can access all relevant information about a texture (e.g. width, height) by calling @ref BansheeEngine::Texture::getProperties() "Texture::getProperties()" which will return an instance of @ref BansheeEngine::TextureProperties "TextureProperties". 
 
# Reading/writing {#textures_c}
To read and write from/to the texture use the @ref BansheeEngine::Texture::readSubresource "Texture::readSubresource" and @ref BansheeEngine::Texture::writeSubresource "Texture::writeSubresource" methods. These expect an index of a sub-resource to read/write to, and a @ref BansheeEngine::PixelData "PixelData" object.

The sub-resource index is simply a sequential index of the surface you want to access. If the texture has only one surface this will always be zero. But if the texture has multiple mipmaps or multiple faces (like for cube textures or texture arrays) you can use @ref BansheeEngine::TextureProperties::mapToSubresourceIdx "TextureProperties::mapToSubresourceIdx" to convert mip/face combination into an subresource index, and @ref BansheeEngine::TextureProperties::mapFromSubresourceIdx "TextureProperties::mapFromSubresourceIdx" for the other way around.

@ref BansheeEngine::PixelData "PixelData" object is just a container for a set of pixels. You can create one manually or use @ref BansheeEngine::TextureProperties::allocateSubresourceBuffer "TextureProperties::allocateSubresourceBuffer" to create the object of valid size and format for the specified sub-resource index. When reading from the texture the buffer will be filled with pixels from the texture, and when writing you are expected to populate the object.

## PixelData {#textures_c_a}
You can create @ref BansheeEngine::PixelData "PixelData" manually by calling @ref BansheeEngine::PixelData::create "PixelData::create" and providing it with dimensions and pixel format. When working with textures you must ensure that the dimensions and the format matches the texture sub-resource.

Once created you can use @ref BansheeEngine::PixelData::getColorAt "PixelData::getColorAt", @ref BansheeEngine::PixelData::setColorAt "PixelData::setColorAt", @ref BansheeEngine::PixelData::getColors "PixelData::getColors" and @ref BansheeEngine::PixelData::setColors "PixelData::setColors" to read/write colors from/to its internal buffer.

You can also use @ref BansheeEngine::PixelUtil "PixelUtil" to perform various operations on the pixels. This includes generating mip maps, converting to different pixel formats, compressing, scaling and other.

## Cached CPU data {#textures_c_b}
When you read from a texture using the @ref BansheeEngine::Texture::readSubresource "Texture::readSubresource" method the read will be performed from the GPU. This is useful if the GPU has in some way modified the texture, but will also incur a potentially large performance penalty because it will introduce a CPU-GPU synchronization point. In a lot of cases you might just want to read pixels from a texture that was imported or created on the CPU in some other way.

For this reason @ref BansheeEngine::Texture::readData "Texture::readData" exists. It will read data quickly with little performance impact. However you must create the texture using the @ref BansheeEngine::TU_CPUCACHED "TU_CPUCACHED" usage. This also means that the texture will keep a copy of its pixels in system memory, so use it sparingly. If the texture is modified from the GPU this method will not reflect such changes.

# Rendering using the texture {#textures_d}
To use a texture for rendering you need to either:
 - (High level) Assign it to a @ref BansheeEngine::Material "Material" which will then automatically get used on a @ref BansheeEngine::Renderable "Renderable" which uses the material. Read the [material manual](@ref materials) for more information.
 - (Low level) Bind the texture to a @ref BansheeEngine::GpuParams "GpuParams" object, which can then be assigned to pipeline for rendering. Read the [render API manual](@ref renderAPI) for more information.

# Saving/loading {#textures_e}
A texture is a @ref BansheeEngine::Resource "Resource" and can be saved/loaded like any other. See the [resource](@ref resources) manual.

# Core thread textures {#textures_f}
So far we have only talked about the simulation thread @ref BansheeEngine::Texture "Texture" but have ignored the core thread @ref BansheeEngine::TextureCore "TextureCore". The functionality between the two is mostly the same, with the major difference being that the core thread version doesn't require a @ref BansheeEngine::CoreThreadAccessor "CoreAccessor" for access, and you can instead perform operations on it directly.

You can also use @ref BansheeEngine::TextureCore::lock "TextureCore::lock" and @ref BansheeEngine::TextureCore::unlock "TextureCore::unlock" to get access to the texture buffer, which allows you to only read/write from/to portions of it, instead of always writing to the entire buffer.

And finally @ref BansheeEngine::TextureCore::copy "TextureCore::copy" method can be used for quickly copying a contents of one texture to another texture. This method will also resolve multi-sampled surface in the case the source is multi-sampled but the destination is not.

# Load-store textures {#textures_g}
Load-store textures are a special type of textures that can be written to by the GPU. This is opposed to normal textures which are read only. They are particularily useful for compute operations which cannot use render targets for output, or for GPU operations that which to write to arbitrary locations rather than just to their own pixel location.

To create a load-store texture just provide the @ref BansheeEngine::TU_LOADSTORE "TU_LOADSTORE" usage flag on creation - the rest of the creation procedure is identical. Load-store textures cannot be multisampled and cannot be used as render or depth-stencil targets. They also cannot have mip-maps nor can they be created with compressed texture formats.

Rendering using these textures is similar to normal textures, but when binding them to @ref BansheeEngine::Material "Material" or @ref BansheeEngine::GpuParams "GpuParams" they also require a separate @ref BansheeEngine::TextureSurface "TextureSurface" parameter to determine the surface of the texture to bind, in case it has multiple surfaces or mip levels.