---
title: Textures
---

Textures are images that are applied to meshes in order to achieve greater surface detail. In bs::f they are represented with the @bs::Texture class. A texture is a resource, meaning it can be imported, saved and loaded as any other resource.

# Importing a texture
Textures can be imported from various third party formats, using the importer.

~~~~~~~~~~~~~{.cpp}
// Import a texture named "myTexture.jpg" from the disk
HTexture texture = gImporter().import<Texture>("myTexture.jpg");
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
 - TARGA
 - TIFF
 
# Creating a texture
Textures can also be created manually, which we cover later in the [creating textures](@ref advancedTextures) manual.
   
# Texture properties
Once a texture has been imported, you can retrieve its properties like width, height and format by calling @bs::Texture::getProperties, which returns a @bs::TextureProperties object.

~~~~~~~~~~~~~{.cpp}
// Retrieve and print out various texture properties
auto& props = texture->getProperties();

gDebug().logDebug("Width: " + toString(props.getWidth()));
gDebug().logDebug("Height: " + toString(props.getHeight()));
gDebug().logDebug("Format: " + toString(props.getFormat()));
gDebug().logDebug("Num. mip maps: " + toString(props.getNumMipmaps()));
~~~~~~~~~~~~~

# Customizing import
Texture import can be customized by providing a @bs::TextureImportOptions object to the importer.

~~~~~~~~~~~~~{.cpp}
auto importOptions = TextureImportOptions::create();
// Set required options here (as described below)

HTexture texture = gImporter().import<Texture>("myTexture.jpg", importOptions);
~~~~~~~~~~~~~

A variety of properties can be customized on import, the most important of which being image format, mip-map generation, sRGB state and caching.

## Image format
@bs::TextureImportOptions::format allows you to specify which format should the texture pixels be in after import. Any of the formats provided in @bs::PixelFormat are supported.

~~~~~~~~~~~~~{.cpp}
// Set format as uncompressed RGB with an alpha channel
importOptions->format = PF_R8G8B8A8;
~~~~~~~~~~~~~

Formats can be split into two main categories:
 - Uncompressed - Contain raw texture data with no quality loss, but with high memory cost. Great for textures used on 2D elements like GUI or sprites, for which compressed formats normally introduce visible artifacts.
 - Compressed - Compressed texture data with a slight quality loss, but with considerably smaller (~4x) memory cost compared to raw texture data. Great for textures used on 3D surfaces in the scene.

Some of most common formats are:
 - PF_R8G8B8 - Uncompressed RGB data, without an alpha channel. 
 - PF_R8G8B8A8 - Uncompressed RGB data, with an alpha channel. 
 - PF_BC1 - Compressed RGB data, without an alpha channel. 
 - PF_BC3 - Compressed RGB data, with an alpha channel. 
 - PF_BC5 - Compressed 2-channel format. 
 
## Mip-maps
@bs::TextureImportOptions::generateMips allows you to specify should mip-maps for the texture be generated. Mipmaps ensure that textures applied to 3D surfaces look good when they're viewed from far away (i.e. when the pixel size they take up on screen is considerably less than the actual texture resolution).

Enabling mip-maps will increase the memory use of the texture by 33%, but it is pretty much essential for any texture applied on a 3D object. It should not be enabled for textures used on 2D elements like GUI or sprites, as it will have no benefit.

~~~~~~~~~~~~~{.cpp}
// Enable mipmap generation
importOptions->generateMips = true;
~~~~~~~~~~~~~

## Gamma correction
Most images output by modern cameras and image editing software will be stored in what's called a gamma (sRGB) space. This ensures that the image has more quality in the blacks, but it does not represent the actual image you see on the screen (when you open it in some image viewing program). This is because your monitor performs gamma-correction on the image, before displaying it to you.

However game engines don't like their data in gamma space, because it's not linear which means that all lighting and similar operations don't operate exactly as they should. Essentially the image ends up looking more "washed-out" than it should.

For this reason bs::f provides @bs::TextureImportOptions::sRGB toggle, which you can set for images in gamma (sRGB) space. The engine will then know to convert to linear space before performing any relevant operations, ensuring the final rendering looks good.

~~~~~~~~~~~~~{.cpp}
// Tell the engine this image is in gamma space
importOptions->sRGB = true;
~~~~~~~~~~~~~

> How do you know when an image is in gamma space and when in linear space? As a general rule an albedo (diffuse) images are in gamma space, and normal maps are in linear space. For other types of images you need to consult the tool you use to create/save them, but in most cases they will be in gamma space.

## Caching
Sometimes you need to import a texture you don't want to only use for rendering, but rather for manually reading its contents. When that's the case you can enable the @bs::TextureImportOptions::cpuCached option.

This will allow you to call @bs::Texture::readCachedData and to manually read individual pixels of the texture.

Note that caching a texture means its data will be available in system memory, essentially doubling its memory usage.

~~~~~~~~~~~~~{.cpp}
// Enable caching
importOptions->cpuCached = true;

// Import texture
HTexture texture = gImporter().import<Texture>("myTexture.jpg", importOptions);

// Allocate a buffer to hold texture contents
SPtr<PixelData> pixelData = PixelData::create(1024, 1024, 1, PF_R8G8B8A8);

// Read cached data
texture->readCachedData(*pixelData);

// Read pixels
Color color = pixelData->getColorAt(50, 50);
...
~~~~~~~~~~~~~

> **PixelData** is explained later in the [creating textures](@ref advancedTextures) manual.