Importing textures 						{#importingTextures}
===============

Textures are images that are applied to meshes in order to achieve greater surface detail. In Banshee they are represented with the @ref bs::Texture "Texture" class. A texture is a resource, meaning it can be imported, saved and loaded as we described in Resource manuals.

![Mesh without (left) and with (right) a texture](TexturedMesh.png)  

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
 
# Customizing import
Texture import can be customized by providing a @ref bs::TextureImportOptions "TextureImportOptions" object to the importer.

~~~~~~~~~~~~~{.cpp}
auto importOptions = TextureImportOptions::create();
// Set required options here (as described below)

HTexture texture = gImporter().import<Texture>("myTexture.jpg", importOptions);
~~~~~~~~~~~~~

A variety of properties can be customized on import, the most important of which being image format, mip-map generation, sRGB state and caching.

## Image format
// TODO

## Mip-maps
// TODO

## sRGB
// TODO

## Caching
// TODO

# Texture properties
Once a texture has been imported, you can retrieve its properties like width, height and format by calling @ref bs::Texture::getProperties "Texture::getProperties()", which returns a @ref bs::TextureProperties "TextureProperties" object.

~~~~~~~~~~~~~{.cpp}
// Retrieve and print out various texture properties
auto& props = texture->getProperties();

gDebug().logDebug("Width: " + toString(props.getWidth()));
gDebug().logDebug("Height: " + toString(props.getHeight()));
gDebug().logDebug("Format: " + toString(props.getFormat()));
gDebug().logDebug("Num. mip maps: " + toString(props.getNumMipmaps()));
~~~~~~~~~~~~~

# Rendering using textures
Once imported a texture can be assigned to a **Material** in order to be used for rendering. We will cover materials in a later chapter.