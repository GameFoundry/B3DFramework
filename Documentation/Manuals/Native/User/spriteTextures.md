Sprite textures									{#spriteTextures}
===============

Before we get started with GUI, let's first introduce the concept of sprite textures. They are very similar to normal textures, with the main difference being that they reference a specific sub-area on a **Texture**. This means multiple sprite textures can reference different parts of a single texture.

They are used primarily by 2D elements, like GUI or sprites. Their primary purpose is to improve performance by ensuring 2D elements can be batched together when rendering, since all batched elements need to use the same underlying texture (also known as a texture atlas).

They are represented with the @ref bs::SpriteTexture "SpriteTexture" class and are a **Resource**, same as normal textures. 

They're created by calling @ref bs::SpriteTexture::create "SpriteTexture::create". As a parameter it expects the source **Texture**, and an optional set of UV coordinates that map to a specific area on the texture. If no coordinates are provided the sprite texture maps to the entirety of the texture, acting the same as a normal texture.

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

Once created, you can get the actual width/height of the mapped area by calling @ref bs::SpriteTexture::getWidth "SpriteTexture::getWidth" and @ref bs::SpriteTexture::getHeight "SpriteTexture::getHeight".

~~~~~~~~~~~~~{.cpp}
// If our original texture was 1024x1024, this will be 512x512, since it's just a
// single quadrant (quarter of the size)
UINT32 width = spriteTexPartial->getWidth();
UINT32 height = spriteTexPartial->getHeight();
~~~~~~~~~~~~~

You can also always retrieve the underlying texture by calling @ref bs::SpriteTexture::getTexture "SpriteTexture::getTexture".

~~~~~~~~~~~~~{.cpp}
HTexture texture = spriteTexPartial->getTexture();
~~~~~~~~~~~~~