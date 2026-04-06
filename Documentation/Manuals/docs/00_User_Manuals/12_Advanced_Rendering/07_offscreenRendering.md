---
title: Offscreen rendering
---

When we talked about how to set up a **Camera** component we have shown that we need to provide it with a render target onto which to output the rendered pixels. In that example we have used a **RenderWindow** as the target, but you may instead also use a @b3d::RenderTexture. Both render windows and render textures derive from a common @b3d::RenderTarget base class, and can be used interchageably in most places.

We call rendering to a texture offscreen rendering. By rendering offscreen you can achieve advanced graphical effects by manipulating the contents of the rendered-to texture before presenting them to the user.

# Creation
Render texture must contain at least one color surface, and may optionally also contain a depth-stencil surface. Both of those surfaces are **Texture** objects, created with either **TextureUsageFlag::RenderTarget** or **TextureUsageFlag::DepthStencil** usage flags, respectively, which we mentioned earlier.

To create a render texture call @b3d::RenderTexture::Create(const RenderTextureCreateInformation&) with a populated @b3d::RenderTextureCreateInformation structure. This structure expects a reference to one or more color surface textures, and an optional depth-stencil surface texture. For each of those you must also specify the face and mip level onto which to render, in case your texture has multiple.

~~~~~~~~~~~~~{.cpp}
// Create a 1920x1080 texture with 32-bit RGBA format
TextureCreateInformation colorCreateInformation;
colorCreateInformation.Type = TEX_TYPE_2D;
colorCreateInformation.Width = 1920;
colorCreateInformation.Height = 1080;
colorCreateInformation.Format = PF_R8G8B8A8;
colorCreateInformation.Usage = TextureUsageFlag::RenderTarget;

HTexture colorTexture = Texture::Create(colorCreateInformation);

// Create a 1920x1080 texture with a 32-bit depth-stencil format
TextureCreateInformation depthStencilCreateInformation;
depthStencilCreateInformation.Type = TEX_TYPE_2D;
depthStencilCreateInformation.Width = 1920;
depthStencilCreateInformation.Height = 1080;
depthStencilCreateInformation.Format = PF_D32;
depthStencilCreateInformation.Usage = TextureUsageFlag::DepthStencil;

HTexture depthStencilTexture = Texture::Create(depthStencilCreateInformation);

RenderTextureCreateInformation renderTextureCreateInformation;
renderTextureCreateInformation.ColorSurfaces[0].Texture = colorTexture;
renderTextureCreateInformation.ColorSurfaces[0].Face = 0;
renderTextureCreateInformation.ColorSurfaces[0].MipLevel = 0;

renderTextureCreateInformation.DepthStencilSurface.Texture = depthStencilTexture;
renderTextureCreateInformation.DepthStencilSurface.Face = 0;
renderTextureCreateInformation.DepthStencilSurface.MipLevel = 0;

SPtr<RenderTexture> renderTexture = RenderTexture::Create(renderTextureCreateInformation);
~~~~~~~~~~~~~

## Multiple surfaces
Render textures can also contain multiple color surfaces (up to 8). Such targets allow you to write more data at once in your shader program. To create a texture with multiple color surfaces simply fill out other entries of @b3d::RenderTextureCreateInformation::ColorSurfaces array and proceed the same as in the above example.

All color surfaces and the depth/stencil surface (if present) must have the same dimensions and sample count.

## Multi-sampled surfaces
Render textures can be created with support for multiple samples per pixel. This allows affects such as multi-sampled antialiasing and similar. To create a multi-sampled render texture simply create a **Texture** with its @b3d::TextureCreateInformation::SampleCount parameter larger than one, which you then use to initialize a render texture. Make sure that all surfaces (including depth-stencil) in a render texture have the same number of samples.

Multisampled textures cannot be used directly by materials or sampled in shaders. This means that before you can use such a texture for normal rendering you must first resolve its multi-sampled contents into a non-multisampled texture. You may do this in two ways:
 - Call @b3d::render::Texture::Copy with the source texture being your multisampled texture, and the destination being a texture of same dimensions and format, but with a single sample per pixel. Note this is a core-thread only method - we talk more about the core thread later.
 - Write a custom shader that manually reads samples from the texture and outputs pixels (out of the scope of this manual)

# Rendering to textures
To render to a render texture you can assign it to a **Viewport** that's part of a **Camera** component, or you may use the low-level API to directly bind the texture for rendering (see the low level rendering manuals).

~~~~~~~~~~~~~{.cpp}
HSceneObject cameraSceneObject = SceneObject::Create("Camera");
HCamera camera = cameraSceneObject->AddComponent<Camera>();
camera->GetViewport()->SetTarget(renderTexture);
~~~~~~~~~~~~~

# Using render textures as input
Once you have performed some rendering into a render texture, you can access its underlying textures by calling @b3d::RenderTexture::GetColorTexture and @b3d::RenderTexture::GetDepthStencilTexture. Once you have the underlying textures you can use them as you would normal textures, i.e. by binding them to a material for rendering, or reading their contents.

~~~~~~~~~~~~~{.cpp}
// Do some rendering to the render texture
HTexture colorTexture = renderTexture->GetColorTexture(0);

// Bind the result as input to a material to render with
HMaterial someMaterial = ...;
someMaterial->SetTexture("gInputTex", colorTexture);
~~~~~~~~~~~~~

You can also query the depth/stencil texture to check the depth information:

~~~~~~~~~~~~~{.cpp}
HTexture depthStencilTexture = renderTexture->GetDepthStencilTexture();
B3D_LOG(Info, LogGeneric, "Depth/stencil texture format: {0}", depthStencilTexture->GetProperties().Format);
~~~~~~~~~~~~~

Please note that a render texture must not be bound for rendering at the same time you are trying to read from it (either from shader of from the CPU). This will result in undefined behaviour.

# Priority
All render targets have a priority that can be set by calling @b3d::RenderTarget::SetPriority. This priority can be used as a hint to the renderer in which order should the targets be rendered to. Targets with higher priority will be rendered to before targets with lower priority. This value is only used for render targets assigned to **Camera**%s, and this value is ignored if rendering using the low-level rendering API as in that case you have manual control over rendering order.

~~~~~~~~~~~~~{.cpp}
renderTexture->SetPriority(50);

// Query the current priority
i32 currentPriority = renderTexture->GetProperties().Priority;
B3D_LOG(Info, LogGeneric, "Render texture priority: {0}", currentPriority);
~~~~~~~~~~~~~
