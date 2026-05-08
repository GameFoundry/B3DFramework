---
title: Advanced textures
---

In this manual we'll learn how to create textures manually, modify their contents and even read-back texture data that was written on the GPU.

# Creating textures
To create a texture call @b3d::Texture::Create. You'll need to populate the @b3d::TextureCreateInformation structure and pass it as a parameter. The structure requires you to populate these properties at minimum:
 - @b3d::TextureInformation::Type - Allows you to choose between 1D/2D/3D or cube-map textures using the @b3d::TextureType enum
 - @b3d::TextureInformation::Format - Allows you to choose a format for each individual pixel in the texture, using the @b3d::PixelFormat enum
 - @b3d::TextureInformation::Width - Width of the texture, in pixels
 - @b3d::TextureInformation::Height - Height of the texture, in pixels. Not relevant for 1D textures.
 - @b3d::TextureInformation::Depth - Depth of the texture, in pixels. Only relevant for 3D textures.

When it comes to texture types there four kinds of textures:
 - @b3d::TEX_TYPE_1D - A one-dimensional texture, containing just one row of pixels. Rarely used.
 - @b3d::TEX_TYPE_2D - A two-dimensional texture, most commonly used type.
 - @b3d::TEX_TYPE_3D - A three-dimensional texture, can be imagined as an array of 2D textures, but with support for interpolating between the texture slices. Used primarily when working with the low-level rendering API for special purposes.
 - @b3d::TEX_TYPE_CUBE_MAP - A cubemap texture that consists out of six 2D textures, each representing one face of the cube. Used primarily when working with the low-level rendering API for special purposes.

You may also set these optional properties:
 - @b3d::TextureInformation::MipMapCount - A texture with mip-maps will contain a set of scaled down versions of itself that are used by the GPU for anti-aliasing. Specify zero to use no mip maps. You can use the helper function @b3d::PixelUtility::GetMipmapCount to return the maximum possible mip-map count for a specific set of dimensions.
 - @b3d::TextureInformation::ArraySliceCount - Specify number higher than 1 in order to create an array of textures. This is primarily used for low-level rendering purposes. Texture arrays are not supported for 3D textures.
 - @b3d::TextureInformation::UseHardwareSRGB - When true, it specifies if the data in the texture is gamma corrected. When performing reads on such texture in a shader the GPU will transform the texture data back to linear space before returning the value. When a texture is used as a render target, the GPU will automatically convert from linear space into gamma space when rendering to the texture. Only relevant for 2D textures.
 - @b3d::TextureInformation::SampleCount - Specifies the number of samples per pixel. This is used primarily for multi-sample antialiasing. This is only relevant for 2D textures, and only for textures used as render targets. You cannot read or write from/to multi-sample textures manually.
 - @b3d::TextureInformation::Usage - Flags that control how is the texture allowed to be used, represented by the @b3d::TextureUsage enum

Supported textures usages are:
 - @b3d::TextureUsageFlag::StoreOnGPU - Specify for normal textures that are created once (or updated very rarely) and used for normal rendering.
 - @b3d::TextureUsageFlag::StoreOnCPUWithGPUAccess - Specify for textures that are updated often (e.g. every frame) and used for normal rendering.
 - @b3d::TextureUsageFlag::RenderTarget - Specify for textures that will be used as color attachments for a render target.
 - @b3d::TextureUsageFlag::DepthStencil - Specify for textures that will be used as a depth/stencil attachment for render target. Texture's pixel format must be one of the depth-stencil formats.
 - @b3d::TextureUsageFlag::AllowUnorderedAccessOnTheGPU - Specify that the texture can be used for random read/write by shaders (e.g. for use in a compute shader).
 - @b3d::TextureUsageFlag::CPUCached - Specify that any data written to the texture (from the CPU) will be cached internally, allowing it to be accessed through **Texture::ReadCachedData()**. Uses extra memory as data needs to be stored in both normal and GPU memory.

Most of these options are only useful when using the low-level rendering API and your own shaders, in which case you might require advanced texture types and options. In majority of cases however you will be using 2D textures with mip-maps, potentially with gamma-corrected data, used for normal rendering (e.g. assigning them to materials).

~~~~~~~~~~~~~{.cpp}
// Creates a 2D texture, 128x128 with an 8-bit RGBA format
TextureCreateInformation textureCreateInfo;
textureCreateInfo.Type = TEX_TYPE_2D;
textureCreateInfo.Width = 128;
textureCreateInfo.Height = 128;
textureCreateInfo.Format = PF_RGBA8;

HTexture texture = Texture::Create(textureCreateInfo);
~~~~~~~~~~~~~

> Low level rendering API is explained as a part of the developer manuals.

# Writing data
Once a texture has been created you might want to write some data to it. This is accomplished by calling @b3d::Texture::WriteData. The method accepts a @b3d::PixelData object, as well as a mip-map level and a face to write to.

**PixelData** object is just a container for all the pixels of a single mip-level & face of a texture. It is created by calling @b3d::PixelData::Create and providing dimensions as well as the pixel format.

~~~~~~~~~~~~~{.cpp}
// Create pixel data for a 128x128 texture with an 8-bit RGBA format
TShared<PixelData> pixelData = PixelData::Create(128, 128, 1, PF_RGBA8);
~~~~~~~~~~~~~

Once created you can set the color of each pixel by calling @b3d::PixelData::SetColorAt, or set all colors at once by calling @b3d::PixelData::SetColors.

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> pixelData = PixelData::Create(128, 128, 1, PF_RGBA8);

// Generate some arbitrary colors
Vector<Color> colors;
for(u32 y = 0; y < 128; y++)
	for(u32 x = 0; x < 128; x++)
		colors.push_back(Color(x / 128.0f, y / 128.0f, 0.0f, 1.0f));

pixelData->SetColors(colors);
~~~~~~~~~~~~~

Finally you can write the data to the texture. Note that this is an asynchronous operation - the function returns immediately but the actual transfer happens later on the GPU.

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...;
TShared<PixelData> pixelData = ...;

TAsyncOp<void> asyncOp = texture->WriteData(pixelData);

// Optionally wait for completion
asyncOp.BlockUntilComplete();
~~~~~~~~~~~~~

## Writing to sub-resources
If a texture contains mip levels, or more than one face then we say it has multiple sub-resources. Each such sub-resource must be written to with a separate call to **Texture::WriteData()**.

A texture has multiple mip-levels if its **TextureInformation::MipMapCount** property is larger than zero.
A texture has multiple faces if its **TextureInformation::ArraySliceCount** property is larger than one, or if texture type is **TextureType::TEX_TYPE_CUBE_MAP**.

If texture type is **TextureType::TEX_TYPE_CUBE_MAP** then the texture will have six faces by default. If such texture has multiple array slices the total number of faces is 6 * number of array slices. Each face has its own set of mip-map levels (if mip-maps count is larger than zero).

To write to different sub-resources simply provide the mip-level and face index when calling **Texture::WriteData()**. Note that when setting mip-levels your **PixelData** object must be of valid size for the mip-level. You can use the helper method @b3d::PixelUtility::GetSizeForMipLevel to calculate dimensions of a specific mip level.

~~~~~~~~~~~~~{.cpp}
// Creates a 2D texture, 128x128 with an 8-bit RGBA format, with maximum number of mipmaps, and 4 faces
TextureCreateInformation textureCreateInfo;
textureCreateInfo.Type = TEX_TYPE_2D;
textureCreateInfo.Width = 128;
textureCreateInfo.Height = 128;
textureCreateInfo.MipMapCount = PixelUtility::GetMipmapCount(128, 128, 1, PF_RGBA8);
textureCreateInfo.Format = PF_RGBA8;
textureCreateInfo.ArraySliceCount = 4;

HTexture texture = Texture::Create(textureCreateInfo);

// For each face, and for each mip-level, write some data
for(u32 face = 0; face < 4; face++)
	for(u32 mipLevel = 0; mipLevel <= textureCreateInfo.MipMapCount; mipLevel++)
	{
		u32 mipWidth, mipHeight, mipDepth;
		PixelUtility::GetSizeForMipLevel(128, 128, 1, mipLevel, mipWidth, mipHeight, mipDepth);

		TShared<PixelData> pixelData = PixelData::Create(mipWidth, mipHeight, 1, PF_RGBA8);

		Vector<Color> colors;
		for(u32 y = 0; y < mipHeight; y++)
			for(u32 x = 0; x < mipWidth; x++)
				colors.push_back(Color(x * 2.0f, y * 2.0f, 0.0f, 1.0f));

		pixelData->SetColors(colors);
		texture->WriteData(pixelData, face, mipLevel);
	}
~~~~~~~~~~~~~

> Note when writing we access a total of *textureCreateInfo.MipMapCount + 1* mip-levels. This is because the 0th mip level is the main texture.

As a shortcut, you can retrieve texture properties by calling **Texture::GetProperties()** and then call the @b3d::TextureProperties::AllocBuffer to automatically create a **PixelData** object of a valid size and format, depending on provided face and mip level.

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...;

const TextureProperties& textureProperties = texture->GetProperties();

// Get buffer with enough space and valid format for 0th face and 2nd mip-level
TShared<PixelData> pixelData = textureProperties.AllocBuffer(0, 2);
// ... populate the buffer and write
~~~~~~~~~~~~~

## Discard on write
When you are sure you will overwrite all the contents of a texture, make sure to set the last parameter of **Texture::WriteData()** to true. This ensures the system can more optimally execute the transfer, without requiring the GPU to finish its current action (which can be considerably slow if it is currently using that particular texture).

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...;
TShared<PixelData> pixelData = ...;

// Discard existing texture contents for better performance
texture->WriteData(pixelData, 0, 0, true);
~~~~~~~~~~~~~

## Generating mip-maps
Mip-maps are generally created automatically from a source texture, rather than by manually setting their pixels. Therefore the framework provides @b3d::PixelUtility::GenerateMipmaps method that accepts a **PixelData** object containing pixels to generate mip levels from. A maximum number of mip-maps levels is then generated and output. You can optionally customize mip-map generation by providing a @b3d::MipMapGenOptions object.

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> sourcePixelData = PixelData::Create(128, 128, 1, PF_RGBA8);
// ... fill sourcePixelData with some colors

Vector<TShared<PixelData>> mipMapPixelDataArray = PixelUtility::GenerateMipmaps(sourcePixelData, MipMapGenOptions());

// Write mipmap data to texture...
HTexture texture = ...;
for(u32 mipLevel = 0; mipLevel < mipMapPixelDataArray.size(); mipLevel++)
{
	texture->WriteData(mipMapPixelDataArray[mipLevel], 0, mipLevel);
}
~~~~~~~~~~~~~

## Writing compressed data
If a **PixelFormat** chosen for your texture uses one of the compressed pixel formats, you will need to compress the data before writing it to the texture. For this purpose you can use the @b3d::PixelUtility::Compress method. The method accepts a source **PixelData** and a destination **PixelData**, as well as a @b3d::CompressionOptions object that contains the pixel format to compress to, among other options.

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> sourcePixelData = PixelData::Create(128, 128, 1, PF_RGBA8);
// ... fill up sourcePixelData with some colors

// Container for resulting data
TShared<PixelData> destinationPixelData = PixelData::Create(128, 128, 1, PF_BC3);

// Compress into BC3 format
CompressionOptions compressionOptions;
compressionOptions.Format = PF_BC3;
PixelUtility::Compress(*sourcePixelData, *destinationPixelData, compressionOptions);

// Write data to texture...
HTexture texture = ...;
texture->WriteData(destinationPixelData);
~~~~~~~~~~~~~

## Creating textures with data
If you're creating a texture you wish to immediately populate with data, you can set the **TextureCreateInformation::InitialData** field to provide a **PixelData** object directly, allowing you to skip the call to **Texture::WriteData()**.

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> pixelData = PixelData::Create(128, 128, 1, PF_RGBA8);
// ... fill up pixelData with some colors

TextureCreateInformation textureCreateInfo;
textureCreateInfo.Type = TEX_TYPE_2D;
textureCreateInfo.Width = 128;
textureCreateInfo.Height = 128;
textureCreateInfo.Format = PF_RGBA8;
textureCreateInfo.InitialData = pixelData;

HTexture texture = Texture::Create(textureCreateInfo);
~~~~~~~~~~~~~

# Reading texture data

Reading data from a texture is done similarly to writing, using **PixelData** object as well. There are two ways to read texture data:
 - Reading cached CPU data
 - Reading GPU data

## Reading cached CPU data
Reading cached CPU data allows you to read-back any data you have written to the texture by calling **Texture::WriteData()**. It is particularily useful when importing textures from external files and wish to access their pixels. Note that texture must be created with the **TextureUsageFlag::CPUCached** usage flag in order for CPU cached data to be available. When importing textures this flag will automatically be set if the relevant property is enabled in **TextureImportOptions**.

Cached CPU data can be read by calling @b3d::Texture::ReadCachedData. It accepts a **PixelData** parameter to which to output the pixel colors, as well as indices to the face & mip-level to read.

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...;
const TextureProperties& textureProperties = texture->GetProperties();

TShared<PixelData> pixelData = textureProperties.AllocBuffer(0, 0);
texture->ReadCachedData(*pixelData);
~~~~~~~~~~~~~

After reading the data you can access it through @b3d::PixelData::GetColorAt or @b3d::PixelData::GetColors.

~~~~~~~~~~~~~{.cpp}
TShared<PixelData> pixelData = ...;

// Read pixel at 50x50
Color color = pixelData->GetColorAt(50, 50);
B3D_LOG(Info, LogGeneric, "Pixel color: {0}", color);

// Read all pixels
Vector<Color> allColors = pixelData->GetColors();
~~~~~~~~~~~~~

Note that cached data reads will not contain any data written by the GPU (e.g. in case the texture is used as a render target or written to by GPU in some other way).

## Reading GPU data
In case cached CPU reads are not enough, you can perform GPU reads, which always read the most recent data which includes both the data written by the CPU and the GPU. Unlike CPU caching this also does not require additional memory to be used to store texture data.

To perform GPU reads call @b3d::Texture::ReadData. It accepts a shared pointer to a **PixelData** object.

~~~~~~~~~~~~~{.cpp}
HTexture texture = ...;
const TextureProperties& textureProperties = texture->GetProperties();

TShared<PixelData> pixelData = textureProperties.AllocBuffer(0, 0);
texture->ReadData(pixelData);
~~~~~~~~~~~~~

Note that performing GPU reads will almost certainly cause a GPU pipeline stall, requiring all GPU operations to finish before the read completes. Such stalls can severely impact performance and should generally be avoided.

This operation is asynchronous. The function returns a @b3d::TAsyncOp object that you can use to track completion. The contents of the provided **PixelData** object will not be populated until the async operation finishes.

~~~~~~~~~~~~~{.cpp}
TAsyncOp<void> asyncOp = texture->ReadData(pixelData);

// Later, check if complete
if (asyncOp.HasCompleted())
{
	// Now pixelData contains the texture contents
	Color color = pixelData->GetColorAt(50, 50);
}
~~~~~~~~~~~~~

Alternatively, you can call @b3d::Texture::ReadData without a PixelData parameter to have the system allocate one for you:

~~~~~~~~~~~~~{.cpp}
TAsyncOp<TShared<PixelData>> asyncOp = texture->ReadData(0, 0); // face 0, mip 0

if (asyncOp.HasCompleted())
{
	TShared<PixelData> pixelData = asyncOp.GetResult();
	Color color = pixelData->GetColorAt(50, 50);
}
~~~~~~~~~~~~~

# Other
Take a look at @b3d::PixelUtility class for a variety of helper methods for manipulating pixel data and colors.
