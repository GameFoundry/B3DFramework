Advanced textures				{#advancedTextures}
===============
[TOC]

In this manual we'll learn how to create textures manually, modify their contents and even read-back texture data that was written on the GPU.

# Creating textures {#advancedTextures_a}
To create a texture call @ref bs::Texture::create "Texture::create()". You'll need to populate the @ref bs::TEXTURE_DESC "TEXTURE_DESC" structure and pass it as a parameter. The structure requires you to populate these properties at minimum:
 - @ref bs::TEXTURE_DESC::type "TEXTURE_DESC::type" - Allows you to choose between 1D/2D/3D or cube-map textures using the @ref bs::TextureType "TextureType" enum
 - @ref bs::TEXTURE_DESC::format "TEXTURE_DESC::format" - Allows you to choose a format for each individual pixel in the texture, using the @ref bs::PixelFormat "PixelFormat" enum
 - @ref bs::TEXTURE_DESC::width "TEXTURE_DESC::width" - Width of the texture, in pixels
 - @ref bs::TEXTURE_DESC::height "TEXTURE_DESC::height" - Height of the texture, in pixels. Not relevant for 1D textures.
 - @ref bs::TEXTURE_DESC::depth "TEXTURE_DESC::depth" - Depth of the texture, in pixels. Only relevant for 3D textures.
 
When it comes to texture types there four kinds of textures:
 - @ref bs::TEX_TYPE_1D "TextureType::TEX_TYPE_1D" - A one-dimensional texture, containing just one row of pixels. Rarely used.
 - @ref bs::TEX_TYPE_2D "TextureType::TEX_TYPE_2D" - A two-dimensional texture, most commonly used type.
 - @ref bs::TEX_TYPE_3D "TextureType::TEX_TYPE_3D" - A three-dimensional texture, can be imagined as an array of 2D textures, but with support for interpolating between the texture slices. Used primarily when working with the low-level rendering API for special purposes.
 - @ref bs::TEX_TYPE_CUBE_MAP "TextureType::TEX_TYPE_CUBE_MAP" - A cubemap texture that consists out of six 2D textures, each representing one face of the cube. Used primarily when working with the low-level rendering API for special purposes.
 
You may also set these optional properties:
 - @ref bs::TEXTURE_DESC::numMips "TEXTURE_DESC::numMips" - A texture with mip-maps will contain a set of scaled down versions of itself that are used by the GPU for anti-aliasing. Specify zero to use no mip maps. You can use the helper function @ref bs::PixelUtil::getMaxMipmaps "PixelUtil::getMaxMipmaps()" to return the maximum possible mip-map count for a specific set of dimensions. 
 - @ref bs::TEXTURE_DESC::numArraySlices "TEXTURE_DESC::numArraySlices" - Specify number higher than 1 in order to create an array of textures. This is primarily used for low-level rendering purposes. Texture arrays are not supported for 3D textures.
 - @ref bs::TEXTURE_DESC::hwGamma "TEXTURE_DESC::hwGamma" - When true, it specifies if the data in the texture is gamma corrected. When performing reads on such texture in a shader the GPU will transform the texture data back to linear space before returning the value. When a texture is used as a render target, the GPU will automatically convert from linear space into gamma space when rendering to the texture. Only relevant for 2D textures.
 - @ref bs::TEXTURE_DESC::numSamples "TEXTURE_DESC::numSamples" - Specifies the number of samples per pixel. This is used primarily for multi-sample antialiasing. This is only relevant for 2D textures, and only for textures used as render targets. You cannot read or write from/to multi-sample textures manually.
 - @ref bs::TEXTURE_DESC::usage "TEXTURE_DESC::usage" - Flags that control how is the texture allowed to be used, represented by the @ref bs::TextureUsage "TextureUsage" enum
 
Supported textures usages are:
 - @ref bs::TU_STATIC "TextureUsage::TU_STATIC" - Specify for normal textures that are created once (or updated very rarely) and used for normal rendering.
 - @ref bs::TU_DYNAMIC "TextureUsage::TU_DYNAMIC" - Specify for textures that are updated often (e.g. every frame) and used for normal rendering.
 - @ref bs::TU_RENDERTARGET "TextureUsage::TU_RENDERTARGET" - Specify for textures that will be used as color attachments for a render target.
 - @ref bs::TU_DEPTHSTENCIL "TextureUsage::TU_DEPTHSTENCIL" - Specify for textures that will be used as a depth/stencil attachment for render target. Texture's pixel format must be one of the depth-stencil formats.
 - @ref bs::TU_LOADSTORE "TextureUsage::TU_LOADSTORE" - Specify that the texture can be used for random read/write by shaders (e.g. for use in a compute shader).
 - @ref bs::TU_CPUCACHED "TextureUsage::TU_CPUCACHED" - Specify that any data written to the texture (from the CPU) will be cached internally, allowing it to be accessed through **Texture::readCachedData()**. Uses extra memory as data needs to be stored in both normal and GPU memory.
 - @ref bs::TU_CPUREADABLE "TextureUsage::TU_CPUREADABLE" - Specify that the CPU should be allowed to read texture data that was written by the GPU (e.g. resulting from rendering or a compute shader writes).
 
Most of these options are only useful when using the low-level rendering API and your own shaders, in which case you might require advanced texture types and options. In majority of cases however you will be using 2D textures with mip-maps, potentially with gamma-corrected data, used for normal rendering (e.g. assigning them to materials).
 
~~~~~~~~~~~~~{.cpp}
// Creates a 2D texture, 128x128 with an 8-bit RGBA format
TEXTURE_DESC desc;
desc.type = TEX_TYPE_2D;
desc.width = 128;
desc.heigth = 128;
desc.format = PF_R8G8B8A8;

HTexture texture = Texture::create(desc);
~~~~~~~~~~~~~

> Low level rendering API is explained as a part of the developer manuals.

# Writing data {#advancedTextures_b}
Once a texture has been created you might want to write some data to it. This is accomplished by calling @ref bs::Texture::writeData "Texture::writeData()". The method accepts a @ref bs::PixelData "PixelData" object, as well as a mip-map level and a face to write to.

**PixelData** object is just a container for all the pixels of a single mip-level & face of a texture. It is created by calling @ref bs::PixelData::create "PixelData::create()" and providing dimensions as well as the pixel format.

~~~~~~~~~~~~~{.cpp}
// Create pixel data for a 128x128 texture with an 8-bit RGBA format
SPtr<PixelData> pixelData = PixelData::create(128, 128, 1, PF_R8G8B8A8);
~~~~~~~~~~~~~

Once created you can set the color of each pixel by calling @ref bs::PixelData::setColorAt "PixelData::setColorAt()", or set all colors at once by calling @ref bs::PixelData::setColors "PixelData::setColors()".

~~~~~~~~~~~~~{.cpp}
// Generate some arbitrary colors
Vector<Color> colors;
for(UINT32 y = 0; y < 128; y++)
	for(UINT32 x = 0; x < 128; x++)
		colors.push_back(Color(x / 128.0f, y / 128.0f, 0.0f, 1.0f));

pixelData->setColors(colors);		
~~~~~~~~~~~~~

Finally you can write the data the the texture.

~~~~~~~~~~~~~{.cpp}
texture->writeData(pixelData);
~~~~~~~~~~~~~

## Writing to sub-resources {#advancedTextures_b_a}
If a texture contains mip levels, or more than one face then we say it has multiple sub-resources. Each such sub-resource must be written to with a separate call to **Texture::writeData()**.

A texture has multiple mip-levels if its **TEXTURE_DESC::numMips** property is larger than zero. 
A texture has multiple faces if its **TEXTURE_DESC::numArraySlices** property is larger than one, or if texture type is **TextureType::TEX_TYPE_CUBE_MAP**. 

If texture type is **TextureType::TEX_TYPE_CUBE_MAP** then the texture will have six faces by default. If such texture has multiple array slices the total number of faces is 6 * number of array slices. Each face has its own set of mip-map levels (if mip-maps count is larger than zero).

To write to different sub-resources simply provide the mip-level and face index when calling **Texture::writeData()**. Note that when setting mip-levels your **PixelData** object must be of valid size for the mip-level. You can use the helper method @ref bs::PixelUtil::getSizeForMipLevel "PixelUtil::getSizeForMipLevel()" to calculate dimensions of a specific mip level.

~~~~~~~~~~~~~{.cpp}
// Creates a 2D texture, 128x128 with an 8-bit RGBA format, with maximum number of mipmaps, and 4 faces
TEXTURE_DESC desc;
desc.type = TEX_TYPE_2D;
desc.width = 128;
desc.heigth = 128;
desc.numMips = PixelUtil::getMaxMipmaps(128, 128, 1, PF_R8G8B8A8);
desc.format = PF_R8G8B8A8;
desc.numArraySlices = 4;

HTexture texture = Texture::create(desc);

// For each face, and for each mip-level, write some data
for(UINT32 face = 0; face < 4; face++)
	for(UINT32 mip = 0; mip <= desc.numMips; mip++)
	{
		UINT32 mipWidth, mipHeight, mipDepth;
		PixelUtil::getSizeForMipLevel(128, 128, 1, mip, mipWidth, mipHeight, mipDepth);
		
		SPtr<PixelData> pixelData = PixelData::create(mipWidth, mipHeight, 1, PF_R8G8B8A8);
		
		Vector<Color> colors;
		for(UINT32 y = 0; y < mipHeight; y++)
			for(UINT32 x = 0; x < mipWidth; x++)
				colors.push_back(Color(x * 2.0f, y * 2.0f, 0.0f, 1.0f));

		pixelData->setColors(colors);
		texture->writeData(pixelData, face, mip);
	}
~~~~~~~~~~~~~

> Note when writing we access a total of *desc.numMips + 1* mip-levels. This is because the 0th mip level is the main texture.

As a shortcut, you can retrieve texture properties by calling **Texture::getProperties()** and then call the @ref bs::TextureProperties::allocBuffer "TextureProperties::allocBuffer()" to automatically create a **PixelData** object of a valid size and format, depending on provided face and mip level.

~~~~~~~~~~~~~{.cpp}
auto& texProps = texture->getProperties();

// Get buffer with enough space and valid format for 0th face and 2nd mip-level
SPtr<PixelData> pixelData = texProps.allocBuffer(0, 2);
// ... populate the buffer and write
~~~~~~~~~~~~~

## Discard on write {#advancedTextures_b_b}
When you are sure you will overwrite all the contents of a texture, make sure to set the last parameter of **Texture::writeData()** to true. This ensures the system can more optimally execute the transfer, without requiring the GPU to finish its current action (which can be considerably slow if it is currently using that particular texture).

## Generating mip-maps {#advancedTextures_b_c}
Mip-maps are generally created automatically from a source texture, rather than by manually setting their pixels. Therefore bs::f provides @ref bs::PixelUtil::genMipmaps "PixelUtil::genMipmaps()" method that accepts a **PixelData** object containing pixels to generate mip levels from. A maximum number of mip-maps levels is then generated and output. You can optionally customize mip-map generation by providing a @ref bs::MipMapGenOptions "MipMapGenOptions" object.

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = "...";
Vector<SPtr<PixelData>> mipMapPixelData = PixelUtil::genMipmaps(*pixelData, MipMapGenOptions());

// Write mipmap data to texture...
~~~~~~~~~~~~~

## Writing compressed data {#advancedTextures_b_d}
If a **PixelFormat** chosen for your texture uses one of the compressed pixel formats, you will need to compress the data before writing it to the texture. For this purpose you can use the @ref bs::PixelUtil::compress "PixelUtil::compress()" method. The method accepts a source **PixelData** and a destination **PixelData**, as well as a @ref bs::CompressionOptions "CompressionOptions" object that contains the pixel format to compress to, among other options.

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> srcPixelData = PixelData::create(128, 128, 1, PF_R8G8B8A8);
// ... fill up srcPixelData with some colors

// Container for resulting data
SPtr<PixelData> dstPixelData = PixelData::create(128, 128, 1, PF_BC3);

// Compress into BC3 format
CompressionOptions options;
options.format = PF_BC3;
PixelUtil::compress(srcPixelData, dstPixelData, options);

// Write data to texture...
~~~~~~~~~~~~~

## Creating textures with data {#advancedTextures_b_e}
If you're creating a texture you wish to immediately populate with data, you can use the overload for @ref bs::Texture::create(const SPtr<PixelData>&, int, bool) "Texture::create()" that accepts a **PixelData** object directly, allowing you to skip the call to **Texture::writeData()**.

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = PixelData::create(128, 128, 1, PF_R8G8B8A8);
// ... fill up pixelData with some colors

HTexture texture = Texture::create(pixelData);
~~~~~~~~~~~~~

# Reading texture data {#advancedTextures_c}

Reading data from a texture is done similarly to writing, using **PixelData** object as well. There are two ways to read texture data:
 - Reading cached CPU data
 - Reading GPU data

## Reading cached CPU data {#advancedTextures_c_a}
Reading cached CPU data allows you to read-back any data you have written to the texture by calling **Texture::writeData()**. It is particularily useful when importing textures from external files and wish to access their pixels. Note that texture must be created with the **TextureUsage::TU_CPUCACHED** usage flag in order for CPU cached data to be available. When importing textures this flag will automatically be set if the relevant property is enabled in **TextureImportOptions**.

Cached CPU data can be read by calling @ref bs::Texture::readCachedData "Texture::readCachedData()". It accepts a **PixelData** parameter to which to output the pixel colors, as well as indices to the face & mip-level to read.

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = texProps.allocBuffer(0, 0);
texture->readCachedData(*pixelData);
~~~~~~~~~~~~~

After reading the data you can access it through @ref bs::PixelData::getColorAt "PixelData::getColorAt()" or @ref bs::PixelData::getColors "PixelData::getColors()".

~~~~~~~~~~~~~{.cpp}
// Read pixel at 50x50
Color color = pixelData->getColorAt(50, 50);
~~~~~~~~~~~~~

Note that cached data reads will not contain any data written by the GPU (e.g. in case the texture is used as a render target or written to by GPU in some other way).

## Reading GPU data {#advancedTextures_c_b}
In case cached CPU reads are not enough, you can perform GPU reads, which always read the most recent data which includes both the data written by the CPU and the GPU. Unlike CPU caching this also does not require additional memory to be used to store texture data. Note that texture must be created with the **TextureUsage::TU_CPUREADABLE** usage flag in order for such reads to be available.

To perform GPU reads call @ref bs::Texture::readData "Texture::readData()" which has the same interface as **Texture::readCachedData()**.

~~~~~~~~~~~~~{.cpp}
SPtr<PixelData> pixelData = texProps.allocBuffer(0, 0);
texture->readData(*pixelData);
~~~~~~~~~~~~~

Note that performing GPU reads will almost certainly cause a GPU pipeline stall, requiring all GPU operations to finish before the read completes. Such stalls can severely impact performance and should generally be avoided.

Also note that this operation is asynchronous. This means the function will return immediately, but the actual contents of the provided **PixelData** object will not be populated until the async operation finishes. Read the [mini-manual](@ref asyncMethod) for async operations for more information.

# Other {#advancedTextures_d}
Take a look at @ref bs::PixelUtil "PixelUtil" class for a variety of helper methods for manipulating pixel data and colors.