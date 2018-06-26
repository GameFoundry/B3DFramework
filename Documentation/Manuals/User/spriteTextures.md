Sprite textures									{#spriteTextures}
===============

Before we get started with GUI, let's first introduce the concept of sprite textures. They are very similar to normal textures, with the main difference being that they reference a specific sub-area on a **Texture**. This means multiple sprite textures can reference different parts of a single texture.

This can be used by internal systems to improve rendering performance as rendered objects sharing the same texture can be 'batched' together, reducing rendering overhead. Additionally it can also be used for performing sprite animation, as we will show later.

Sprite textures are used extensively by the GUI system, but are also usable in other systems (e.g. on a **Material** or a **ParticleSystem**).

They are represented with the @ref bs::SpriteTexture "SpriteTexture" class and are a **Resource**, same as normal textures. 

They're created by calling @ref bs::SpriteTexture::create "SpriteTexture::create()". As a parameter it expects the source **Texture**, and an optional set of UV coordinates that map to a specific area on the texture. If no coordinates are provided the sprite texture maps to the entirety of the texture, acting the same as a normal texture.

UV coordinates begin in the top left corner, and are in range [0, 1], where top left is (0, 0), and bottom right (1, 1).

~~~~~~~~~~~~~{.cpp}
HTexture texture = gImporter().import<Texture>("myTexture.jpg");

// Sprite texture covering the entire area of "texture"
HSpriteTexture spriteTexComplete = SpriteTexture::create(texture);

// Sprite texture covering the bottom right quadrant of the "texture"
Vector2 offset(0.5f, 0.5f);
Vector2 size(0.5f, 0.5f);
HSpriteTexture spriteTexPartial = SpriteTexture::create(offset, size, texture)
~~~~~~~~~~~~~

Once created, you can get the actual width/height of the mapped area by calling @ref bs::SpriteTexture::getWidth "SpriteTexture::getWidth()" and @ref bs::SpriteTexture::getHeight "SpriteTexture::getHeight()".

~~~~~~~~~~~~~{.cpp}
// If our original texture was 1024x1024, this will be 512x512, since it's just a
// single quadrant (quarter of the size)
UINT32 width = spriteTexPartial->getWidth();
UINT32 height = spriteTexPartial->getHeight();
~~~~~~~~~~~~~

You can also always retrieve the underlying texture by calling @ref bs::SpriteTexture::getTexture "SpriteTexture::getTexture()".

~~~~~~~~~~~~~{.cpp}
HTexture texture = spriteTexPartial->getTexture();
~~~~~~~~~~~~~

# Animation {#spriteTextures_a}
Sprite textures also support sprite sheet grid based animation. To initialize the animation you need to populate the @ref bs::SpriteSheetGridAnimation "SpriteSheetGridAnimation" structure and pass it along to @ref bs::SpriteTexture::setAnimation "SpriteTexture::setAnimation()".

**SpriteSheetGridAnimation** specifies how are animation frames positioned. All frames are expected to be arranged in a grid where each sprite has the same width/height. You will need to provide the number of grid rows and columns, as well as total number of frames and animation speed in the form of frames per second.

And example sprite sheet with 3x3 grid and a total of 8 frames would look like so:
![Example sprite sheet](SpriteSheet.png)

And its corresponding **SpriteSheetGridAnimation**:

~~~~~~~~~~~~~{.cpp}
SpriteSheetGridAnimation anim;
anim.numColumns = 3;
anim.numRows = 3;
anim.count = 8;
anim.fps = 8; // 1 second for one animation loop

HSpriteTexture spriteTexture = ...;
spriteTexture->setAnimation(anim);
~~~~~~~~~~~~~

Finally you need to enable animation playback by passing one of the @ref bs::SpriteAnimationPlayback "SpriteAnimationPlayback" values to @ref bs::SpriteTexture::setAnimationPlayback "SpriteTexture::setAnimationPlayback()".

~~~~~~~~~~~~~{.cpp}
spriteTexture->setAnimationPlayback(SpriteAnimationPlayback::Loop);
~~~~~~~~~~~~~

Sprite textures can then be passed along to various engine systems and animation will be played automatically if animation playback is supported by that system.