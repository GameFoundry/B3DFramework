---
title: Offscreen rendering
---

When we talked about how to set up a **Camera** component we have shown that we need to provide it with a render target onto which to output the rendered pixels. In that example we have used a **RenderWindow** as the target, but you may instead also use a @bs::RenderTexture. Both render windows and render textures derive from a common @bs::RenderTarget base class, and can be used interchageably in most places. 

We call rendering to a texture offscreen rendering. By rendering offscreen you can achieve advanced graphical effects by manipulating the contents of the rendered-to texture before presenting them to the user. 

# Creation
Render texture must contain at least one color surface, and may optionally also contain a depth-stencil surface. Both of those surfaces are **Texture** objects, created with either **TU_RENDERTARGET** or **TU_DEPTHSTENCIL** usage flags, respectively, which we mentioned earlier. 

To create a render texture call @bs::RenderTexture::create(const RENDER_TEXTURE_DESC&) with a populated @bs::RENDER_TEXTURE_DESC structure. This structure expects a reference to one or more color surface textures, and an optional depth-stencil surface texture. For each of those you must also specify the face and mip level onto which to render, in case your texture has multiple.

~~~~~~~~~~~~~{.cpp}
// Create a 1920x1080 texture with 32-bit RGBA format
TEXTURE_DESC colorDesc;
colorDesc.type = TEX_TYPE_2D;
colorDesc.width = 1920;
colorDesc.heigth = 1080;
colorDesc.format = PF_R8G8B8A8;
colorDesc.usage = TU_RENDERTARGET;

HTexture color = Texture::create(colorDesc);

// Create a 1920x1080 texture with a 32-bit depth-stencil format
TEXTURE_DESC depthDesc;
depthDesc.type = TEX_TYPE_2D;
depthDesc.width = 1920;
depthDesc.heigth = 1080;
depthDesc.format = PF_R8G8B8A8;
depthDesc.usage = TU_DEPTHSTENCIL;

HTexture depthStencil = Texture::create(depthDesc);

RENDER_TEXTURE_DESC desc;
desc.colorSurfaces[0].texture = color;
desc.colorSurfaces[0].face = 0;
desc.colorSurfaces[0].mipLevel = 0;

desc.depthStencilSurface.texture = depthStencil;
desc.depthStencilSurface.face = 0;
desc.depthStencilSurface.mipLevel = 0;

SPtr<RenderTexture> renderTexture = RenderTexture::create(desc);
~~~~~~~~~~~~~

## Multiple surfaces
Render textures can also contain multiple color surfaces (up to 8). Such targets allow you to write more data at once in your shader program. To create a texture with multiple color surfaces simply fill out other entries of @bs::RENDER_TEXTURE_DESC array and proceed the same as in the above example.

All color surfaces and the depth/stencil surface (if present) must have the same dimensions and sample count.

## Multi-sampled surfaces
Render textures can be created with support for multiple samples per pixel. This allows affects such as multi-sampled antialiasing and similar. To create a multi-sampled render texture simply create a **Texture** with its `multisampleCount` parameter larger than one, which you then use to initialize a render texture. Make sure that all surfaces (including depth-stencil) in a render texture have the same number of samples.

Multisampled textures cannot be used directly by materials or sampled in shaders. This means that before you can use such a texture for normal rendering you must first resolve its multi-sampled contents into a non-multisampled texture. You may do this in two ways:
 - Call @bs::ct::Texture::copy with the source texture being your multisampled texture, and the destination being a texture of same dimensions and format, but with a single sample per pixel. Note this is a core-thread only method - we talk more about the core thread later.
 - Write a custom shader that manually reads samples from the texture and outputs pixels (out of the scope of this manual)

# Rendering to textures
To render to a render texture you can assign it to a **Viewport** that's part of a **Camera** component, or you may use the low-level API to directly bind the texture for rendering (see the low level rendering manuals).

~~~~~~~~~~~~~{.cpp}
HSceneObject cameraSO = SceneObject::create("Camera");
HCamera camera = cameraSO->addComponent<CCamera>();
camera->getViewport()->setTarget(renderTexture);
~~~~~~~~~~~~~

# Using render textures as input
Once you have performed some rendering into a render texture, you can access its underlying textures by calling @bs::RenderTexture::getColorTexture and @bs::RenderTexture::getDepthStencilTexture. Once you have the underlying textures you can use them as you would normal textures, i.e. by binding them to a material for rendering, or reading their contents.

~~~~~~~~~~~~~{.cpp}
// Do some rendering to the render texture
HTexture colorTexture = renderTexture->getColorTexture(0);

// Bind the result as input to a material to render with
HMaterial someMaterial = ...;
someMaterial->setTexture("gInputTex", colorTexture);
~~~~~~~~~~~~~

Please note that a render texture must not be bound for rendering at the same time you are trying to read from it (either from shader of from the CPU). This will result in undefined behaviour.

# Priority
All render targets have a priority that can be set by calling @bs::RenderTarget::setPriority. This priority can be used as a hint to the renderer in which order should the targets be rendered to. Targets with higher priority will be rendered to before targets with lower priority. This value is only used for render targets assigned to **Camera**%s, and this value is ignored if rendering using the low-level rendering API as in that case you have manual control over rendering order. This is useful if you are rendering to a texture which is used in a later stage as an input, in which case you can ensure the rendering to the texture happens first.

~~~~~~~~~~~~~{.cpp}
renderTexture->setPriority(50);
~~~~~~~~~~~~~
