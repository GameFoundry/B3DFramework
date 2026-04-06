---
title: Textures
---

Textures are images that are applied to meshes in order to achieve greater surface detail. In the framework they are represented with the @b3d::Texture class. A texture is a resource, meaning it can be imported, saved and loaded as any other resource.

# Importing a texture
Textures can be imported from various third party formats, using the importer.

~~~~~~~~~~~~~{.cpp}
// Import a texture named "myTexture.jpg" from the disk
HTexture texture = GetImporter().Import<Texture>("myTexture.jpg");
~~~~~~~~~~~~~

Supported formats are:
 - BMP
 - DDS
 - EXR
 - GIF
 - HDR
 - ICO
 - JPG
 - PNG
 - PSD
 - RAW
 - TARGA (TGA)
 - TIFF

# Creating a texture
Textures can also be created manually, which we cover later in the [creating textures](../12_Advanced_Rendering/07_advancedTextures.md) manual.

# Texture properties
Once a texture has been imported, you can retrieve its properties like width, height and format by calling @b3d::Texture::GetProperties, which returns a @b3d::TextureProperties object.

~~~~~~~~~~~~~{.cpp}
// Retrieve and print out various texture properties
const auto& properties = texture->GetProperties();

B3D_LOG(Info, LogGeneric, "Width: {0}", properties.GetWidth());
B3D_LOG(Info, LogGeneric, "Height: {0}", properties.GetHeight());
B3D_LOG(Info, LogGeneric, "Format: {0}", (u32)properties.GetFormat());
B3D_LOG(Info, LogGeneric, "Num. mip maps: {0}", properties.GetMipMapCount());
~~~~~~~~~~~~~

Additional texture properties:

~~~~~~~~~~~~~{.cpp}
// Get texture type (2D, 3D, cube map)
TextureType textureType = properties.GetTextureType();

// Get depth (for 3D textures)
u32 depth = properties.GetDepth();

// Get number of array slices (for texture arrays)
u32 arraySliceCount = properties.GetArraySliceCount();

// Get number of samples (for multisampled textures)
u32 sampleCount = properties.GetSampleCount();

// Check if texture uses sRGB
bool isSRGB = properties.UseHardwareSRGB();

// Get usage flags
i32 usage = properties.GetUsage();
~~~~~~~~~~~~~

# Customizing import
Texture import can be customized by providing a @b3d::TextureImportOptions object to the importer.

~~~~~~~~~~~~~{.cpp}
auto importOptions = TextureImportOptions::Create();
// Set required options here (as described below)

HTexture texture = GetImporter().Import<Texture>("myTexture.jpg", importOptions);
~~~~~~~~~~~~~

A variety of properties can be customized on import, the most important of which being image format, mip-map generation, sRGB state and caching.

## Image format
@b3d::TextureImportOptions::Format allows you to specify which format should the texture pixels be in after import. Any of the formats provided in @b3d::PixelFormat are supported.

~~~~~~~~~~~~~{.cpp}
// Set format as uncompressed RGB with an alpha channel
importOptions->Format = PF_RGBA8;
~~~~~~~~~~~~~

Formats can be split into two main categories:
 - Uncompressed - Contain raw texture data with no quality loss, but with high memory cost. Great for textures used on 2D elements like GUI or sprites, for which compressed formats normally introduce visible artifacts.
 - Compressed - Compressed texture data with a slight quality loss, but with considerably smaller (~4x) memory cost compared to raw texture data. Great for textures used on 3D surfaces in the scene.

Some of most common formats are:
 - PF_RGB8 - Uncompressed RGB data, without an alpha channel.
 - PF_RGBA8 - Uncompressed RGB data, with an alpha channel.
 - PF_BC1 - Compressed RGB data, without an alpha channel.
 - PF_BC3 - Compressed RGB data, with an alpha channel.
 - PF_BC5 - Compressed 2-channel format (commonly used for normal maps).
 - PF_BC6H - Compressed HDR format for high dynamic range images.
 - PF_BC7 - High quality compressed format with optional alpha.

## Mip-maps
@b3d::TextureImportOptions::GenerateMips allows you to specify should mip-maps for the texture be generated. Mipmaps ensure that textures applied to 3D surfaces look good when they're viewed from far away (i.e. when the pixel size they take up on screen is considerably less than the actual texture resolution).

Enabling mip-maps will increase the memory use of the texture by 33%, but it is pretty much essential for any texture applied on a 3D object. It should not be enabled for textures used on 2D elements like GUI or sprites, as it will have no benefit.

~~~~~~~~~~~~~{.cpp}
// Enable mipmap generation
importOptions->GenerateMips = true;

// Optionally limit the maximum mip level
importOptions->MaxMip = 4; // Generate up to 4 mip levels (0 = unlimited)
~~~~~~~~~~~~~

## Gamma correction
Most images output by modern cameras and image editing software will be stored in what's called a gamma (sRGB) space. This ensures that the image has more quality in the blacks, but it does not represent the actual image you see on the screen (when you open it in some image viewing program). This is because your monitor performs gamma-correction on the image, before displaying it to you.

However game engines don't like their data in gamma space, because it's not linear which means that all lighting and similar operations don't operate exactly as they should. Essentially the image ends up looking more "washed-out" than it should.

For this reason the framework provides @b3d::TextureImportOptions::SRgb toggle, which you can set for images in gamma (sRGB) space. The engine will then know to convert to linear space before performing any relevant operations, ensuring the final rendering looks good.

~~~~~~~~~~~~~{.cpp}
// Tell the engine this image is in gamma space
importOptions->SRgb = true;
~~~~~~~~~~~~~

> How do you know when an image is in gamma space and when in linear space? As a general rule, albedo (diffuse) images are in gamma space, and normal maps are in linear space. For other types of images you need to consult the tool you use to create/save them, but in most cases they will be in gamma space.

## Cubemap import
Textures can be imported as cubemaps for environment mapping and reflections:

~~~~~~~~~~~~~{.cpp}
// Import as cubemap
importOptions->Cubemap = true;

// Set how the source texture should be interpreted
importOptions->CubemapSourceType = CubemapSourceType::Faces; // Individual faces
// or
importOptions->CubemapSourceType = CubemapSourceType::Single; // Single texture to convert
~~~~~~~~~~~~~

## CPU caching
Sometimes you need to import a texture you don't want to only use for rendering, but rather for manually reading its contents. When that's the case you can enable the @b3d::TextureImportOptions::CpuCached option.

This will allow you to call @b3d::Texture::GetCachedData and to manually read individual pixels of the texture.

Note that caching a texture means its data will be available in system memory, essentially doubling its memory usage.

~~~~~~~~~~~~~{.cpp}
// Enable caching
importOptions->CpuCached = true;

// Import texture
HTexture texture = GetImporter().Import<Texture>("myTexture.jpg", importOptions);

// Allocate a buffer to hold texture contents
SPtr<PixelData> pixelData = PixelData::Create(1024, 1024, 1, PF_RGBA8);

// Read cached data
texture->GetCachedData(*pixelData);

// Read pixels
Color color = pixelData->GetColorAt(50, 50);
~~~~~~~~~~~~~

> **PixelData** is explained in detail in the [creating textures](../12_Advanced_Rendering/07_advancedTextures.md) manual.

# Reading pixel data
When you have CPU-cached texture data, you can read and manipulate individual pixels:

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = texture->AllocatePixelBuffer();
texture->GetCachedData(*pixelData);

// Get pixel at specific coordinates
Color color = pixelData->GetColorAt(100, 200);

B3D_LOG(Info, LogGeneric, "Pixel color: R={0}, G={1}, B={2}, A={3}",
    color.r, color.g, color.b, color.a);

// Set pixel color
pixelData->SetColorAt(100, 200, Color::Red);

// Get raw pixel pointer
u8* pixels = pixelData->GetData();

// Get properties
u32 width = pixelData->GetWidth();
u32 height = pixelData->GetHeight();
u32 depth = pixelData->GetDepth();
PixelFormat format = pixelData->GetFormat();
~~~~~~~~~~~~~

## Working with different pixel formats

~~~~~~~~~~~~~{.cpp}
// For different formats, you may need to work with raw bytes
SPtr<PixelData> pixelData = texture->AllocatePixelBuffer();
texture->GetCachedData(*pixelData);

u32 pixelSize = PixelUtility::GetFormatSize(pixelData->GetFormat());
u32 rowPitch = pixelData->GetRowPitch();

// Access pixel at (x, y)
u32 x = 50, y = 100;
u8* pixelDataRaw = pixelData->GetData() + (y * rowPitch) + (x * pixelSize);

// For RGBA8 format, read individual channels
if (pixelData->GetFormat() == PF_RGBA8)
{
    u8 r = pixelDataRaw[0];
    u8 g = pixelDataRaw[1];
    u8 b = pixelDataRaw[2];
    u8 a = pixelDataRaw[3];
}
~~~~~~~~~~~~~

# Writing texture data
You can write new data to textures. This is an asynchronous operation - the function returns immediately but the actual transfer happens on the GPU.

~~~~~~~~~~~~~{.cpp}
// Create pixel data with new content
SPtr<PixelData> pixelData = PixelData::Create(512, 512, 1, PF_RGBA8);

// Fill with a solid color
for (u32 y = 0; y < 512; y++)
	for (u32 x = 0; x < 512; x++)
		pixelData->SetColorAt(x, y, Color::Blue);

// Write to texture (async operation)
TAsyncOp<void> asyncOp = texture->WriteData(pixelData);

// Optionally wait for completion
asyncOp.BlockUntilComplete();
~~~~~~~~~~~~~

## Writing to specific mip levels and faces

For textures with multiple mip levels or array slices, you can specify which subresource to write to:

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = ...;

// Write to face 0, mip level 2
texture->WriteData(pixelData, 0, 2);

// Write to face 3, mip level 0, discarding previous contents for better performance
texture->WriteData(pixelData, 3, 0, true);
~~~~~~~~~~~~~

# Using builtin textures
The framework provides builtin textures for common use cases:

~~~~~~~~~~~~~{.cpp}
// Pure white 2D texture
HTexture white = GetBuiltinResources().GetBuiltinTexture(BuiltinTexture::White);

// Pure black 2D texture
HTexture black = GetBuiltinResources().GetBuiltinTexture(BuiltinTexture::Black);

// Normal map pointing straight up (RGB: 0.5, 0.5, 1.0)
HTexture normal = GetBuiltinResources().GetBuiltinTexture(BuiltinTexture::Normal);

// White 3D texture
HTexture white3D = GetBuiltinResources().GetBuiltinTexture(BuiltinTexture::White3D);

// Black 3D texture
HTexture black3D = GetBuiltinResources().GetBuiltinTexture(BuiltinTexture::Black3D);
~~~~~~~~~~~~~

Builtin textures are immediately available and don't require importing or loading.

# Texture types
The framework supports different types of textures:

~~~~~~~~~~~~~{.cpp}
// Check texture type
const auto& properties = texture->GetProperties();
TextureType type = properties.GetTextureType();

if (type == TEX_TYPE_2D)
    B3D_LOG(Info, LogGeneric, "2D texture");
else if (type == TEX_TYPE_3D)
    B3D_LOG(Info, LogGeneric, "3D texture");
else if (type == TEX_TYPE_CUBE_MAP)
    B3D_LOG(Info, LogGeneric, "Cube map texture");
else if (type == TEX_TYPE_2D_ARRAY)
    B3D_LOG(Info, LogGeneric, "2D texture array");
else if (type == TEX_TYPE_CUBE_MAP_ARRAY)
    B3D_LOG(Info, LogGeneric, "Cube map texture array");
~~~~~~~~~~~~~

# Texture usage flags
When creating textures programmatically, you can specify usage flags:

~~~~~~~~~~~~~{.cpp}
TextureCreateInformation textureCreateInformation;
textureCreateInformation.Width = 1024;
textureCreateInformation.Height = 1024;
textureCreateInformation.Format = PF_RGBA8;

// Set usage flags
textureCreateInformation.Usage = TextureUsageFlag::StoreOnGPU; // Default, not often updated
// or
textureCreateInformation.Usage = TextureUsageFlag::StoreOnCPUWithGPUAccess; // Frequently updated from CPU
// or
textureCreateInformation.Usage = TextureUsageFlag::RenderTarget; // Can be rendered to by GPU
// or
textureCreateInformation.Usage = TextureUsageFlag::CPUCached; // Keep copy in system memory

HTexture texture = Texture::Create(textureCreateInformation);
~~~~~~~~~~~~~
