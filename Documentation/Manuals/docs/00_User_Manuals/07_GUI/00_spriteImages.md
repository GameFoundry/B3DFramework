---
title: Sprite images
---
Before we get started with GUI, let's first introduce the concept of sprite images. A sprite image is an abstraction over various types of image sources that can be rendered to the screen. The framework provides several implementations that allow you to work with textures, vector paths, and font glyphs through a unified interface.

Sprite images are used extensively by the GUI system, but are also usable in other systems such as particle emitters and materials.

All sprite images inherit from the @b3d::SpriteImage base class and are **Resources**. They can reference sub-areas of textures to enable texture atlasing, which improves rendering performance by allowing objects that share the same texture to be rendered together in batches.

# SpriteTexture

The most common sprite image type is @b3d::SpriteTexture, which wraps a regular **Texture** and optionally references a specific sub-area within it. Multiple sprite textures can reference different parts of a single texture, enabling texture atlas workflows.

Sprite textures are created by calling @b3d::SpriteTexture::Create. You can provide just a texture to map to its entire area, or specify UV coordinates to reference a sub-region.

UV coordinates begin in the top left corner and are in the range [0, 1], where top left is (0, 0) and bottom right is (1, 1).

~~~~~~~~~~~~~{.cpp}
HTexture texture = GetImporter().Import<Texture>("myTexture.jpg");

// Sprite texture covering the entire area of "texture"
HSpriteTexture spriteTextureComplete = SpriteTexture::Create(texture);

// Sprite texture covering the bottom right quadrant of "texture"
SpriteTextureCreateInformation createInformation;
createInformation.AtlasTexture = texture;
createInformation.UVRange = Area2(0.5f, 0.5f, 0.5f, 0.5f); // offset (0.5, 0.5), size (0.5, 0.5)
HSpriteTexture spriteTexturePartial = SpriteTexture::Create(createInformation);
~~~~~~~~~~~~~

You can retrieve the UV range that a sprite texture references by calling @b3d::SpriteTexture::GetUVRange.

~~~~~~~~~~~~~{.cpp}
Area2 uvRange = spriteTexturePartial->GetUVRange();
~~~~~~~~~~~~~

You can also retrieve the underlying atlas texture by calling @b3d::SpriteTexture::GetAtlasTexture.

~~~~~~~~~~~~~{.cpp}
HTexture atlasTexture = spriteTexturePartial->GetAtlasTexture();
~~~~~~~~~~~~~

# SpriteVectorPath

The @b3d::SpriteVectorPath class renders vector graphics paths as sprite images. Vector paths can be scaled to any size while maintaining quality, making them ideal for resolution-independent graphics.

To create a sprite vector path, you need to provide a **VectorPath** resource and specify the default rendering size in pixels.

~~~~~~~~~~~~~{.cpp}
HVectorPath vectorPath = ...; // Load or create a vector path
Size2I defaultSize(64, 64);
HSpriteVectorPath spriteVectorPath = SpriteVectorPath::Create(vectorPath, defaultSize);
~~~~~~~~~~~~~

When rendered at different scales, the vector path will be automatically re-rasterized to maintain optimal quality. For detailed information about creating and working with vector paths, see the [vector shapes](07_vectorShapes.md) manual.

# SpriteGlyph

The @b3d::SpriteGlyph class renders individual font glyphs as sprite images. You specify which glyph to render by providing a **Font** resource, the Unicode code point of the glyph, and the default size in points.

~~~~~~~~~~~~~{.cpp}
HFont font = GetImporter().Import<Font>("myFont.ttf");
u32 glyphCode = 'A'; // Unicode code point for the letter A
float sizeInPoints = 12.0f;
HSpriteGlyph spriteGlyph = SpriteGlyph::Create(font, glyphCode, sizeInPoints);
~~~~~~~~~~~~~

Like vector paths, glyphs can be re-rendered at different scales to maintain quality. For detailed information about importing fonts and working with font resources, see the [fonts](05_fonts.md) manual.

# Image allocations

All sprite images provide a default allocation through @b3d::SpriteImage::GetDefaultAllocatedImage. An allocation represents the actual rendered image data stored in a texture, along with information about where in the texture it resides.

~~~~~~~~~~~~~{.cpp}
HSpriteTexture spriteTexture = SpriteTexture::Create(texture);
const SpriteImageAllocation& allocation = spriteTexture->GetDefaultAllocatedImage();

// Get the texture where the image is stored
HTexture allocatedTexture = allocation.GetTexture();

// Get the UV range within that texture
Area2 uvRange = allocation.GetUVRange();
~~~~~~~~~~~~~

For sprite images that support dynamic scaling (such as **SpriteVectorPath** and **SpriteGlyph**), you can request additional allocations at different sizes or scales through @b3d::SpriteImage::FindOrAllocateImageToFitArea and @b3d::SpriteImage::FindOrAllocateScaledImage.

~~~~~~~~~~~~~{.cpp}
HSpriteVectorPath spriteVectorPath = ...;

// Request an allocation that fits a specific pixel area
Size2I targetSize(128, 128);
TShared<SpriteImageAllocation> scaledAllocation = spriteVectorPath->FindOrAllocateImageToFitArea(targetSize);

// Or request an allocation at a specific scale factor
float scale = 2.0f;
TShared<SpriteImageAllocation> scaledAllocation2 = spriteVectorPath->FindOrAllocateScaledImage(scale);
~~~~~~~~~~~~~

These allocations are cached, so requesting the same size or scale multiple times will return the existing allocation rather than creating a new one. The allocation remains valid as long as you hold a reference to the returned **SpriteImageAllocation** object.

# Animation

All sprite images support sprite sheet grid-based animation. To initialize animation, populate a @b3d::SpriteSheetGridAnimation structure and pass it to @b3d::SpriteImage::SetAnimation.

**SpriteSheetGridAnimation** specifies how animation frames are positioned. All frames are expected to be arranged in a grid where each frame has the same width and height. You need to provide the number of grid rows and columns, the total number of frames, and the animation speed in frames per second.

An example sprite sheet with a 3x3 grid and a total of 8 frames would look like this:
![Example sprite sheet](../../Images/SpriteSheet.png)

And its corresponding **SpriteSheetGridAnimation**:

~~~~~~~~~~~~~{.cpp}
SpriteSheetGridAnimation animation;
animation.RowCount = 3;
animation.ColumnCount = 3;
animation.FrameCount = 8;
animation.FramesPerSecond = 8; // 1 second for one animation loop

HSpriteTexture spriteTexture = ...;
spriteTexture->SetAnimation(animation);
~~~~~~~~~~~~~

Finally, enable animation playback by passing one of the @b3d::SpriteAnimationPlayback values to @b3d::SpriteImage::SetAnimationPlayback.

~~~~~~~~~~~~~{.cpp}
spriteTexture->SetAnimationPlayback(SpriteAnimationPlayback::Loop);
~~~~~~~~~~~~~

Sprite images can then be passed to various engine systems, and animation will be played automatically if animation playback is supported by that system.
