Render targets				{#renderTargets}
===============
[TOC]

Render targets represent destination surfaces onto which objects are rendered. They can be textures or windows.

In Banshee render targets are represented with:
 - Windows - @ref BansheeEngine::RenderWindow "RenderWindow" and @ref BansheeEngine::RenderWindowCore "RenderWindowCore"
 - Textures - @ref BansheeEngine::RenderTexture "RenderTexture" and @ref BansheeEngine::RenderTextureCore "RenderTextureCore"
 
Each type comes in two variants, both of which provide almost equivalent functionality, but the former is for use on the simulation thread, and the latter is for use on the core thread. If you are confused by the dual nature of the objects, read the [core thread](@ref coreThread) manual. 

We're going to focus on the simulation thread implementation in this manual, and then note the differences in the core thread version at the end.

# Render windows {#renderTargets_a}
Render windows are render targets that are backed up by OS-specific windows. Each application must have at least one render window in order to display its contents to the user. All created windows are double or triple buffered, meaning they internally have multiple render surfaces that are swapped between during rendering (see the section about rendering later). This makes them useful for displaying final rendering results in the screen, while render textures are used for intermediary processing.

Creating a @ref BansheeEngine::RenderWindow "RenderWindow" involves filling out the @ref BansheeEngine::RENDER_WINDOW_DESC "RENDER_WINDOW_DESC" and calling @ref BansheeEngine::RenderWindow::create "RenderWindow::create". This method also accepts a parent render window, in case your window is meant to be a child of another window. @ref BansheeEngine::RENDER_WINDOW_DESC "RENDER_WINDOW_DESC" contains many properties, and you should read its reference for information about what all of them do. In this manual we'll just cover the most important settings:

@ref BansheeEngine::RENDER_WINDOW_DESC::videoMode "RENDER_WINDOW_DESC::videoMode"
 - This is the only field that is required, all others are optional. It controls the resolution of the render target, and output monitor and its refresh rate. 

You can create a @ref BansheeEngine::VideoMode "VideoMode" manually by providing a custom resolution and a refresh rate, or you can query @ref BansheeEngine::RenderAPI::getVideoModeInfo "RenderAPI::getVideoModeInfo" to retrieve all available video modes for all available monitors. The latter is the suggested method as it guarantees the video mode will be compatible with the hardware.

@ref BansheeEngine::RENDER_WINDOW_DESC::fullscreen "RENDER_WINDOW_DESC::fullscreen"
 - Controlls whether or not to create the window in fullscreen mode. Fullscreen mode gives your application the exclusive access for rendering device. Without it refresh rate and vsync options are ignored.

@ref BansheeEngine::RENDER_WINDOW_DESC::vsync "RENDER_WINDOW_DESC::vsync"
 - Controls whether the GPU should wait for the vertical sync before presenting the rendered image. This prevents screen tearing. 

@ref BansheeEngine::RENDER_WINDOW_DESC::multisampleCount "RENDER_WINDOW_DESC::multisampleCount"
 - Controls whether you want the render window target to be a multisampled surface. This allows you to perform MSAA antialiasing and similar effects. Most applications will set this to false and instead use multi-sampled render textures, and use the render window for displaying the final resolved (antialiased) image.

@ref BansheeEngine::RENDER_WINDOW_DESC::depthBuffer "RENDER_WINDOW_DESC::depthBuffer"
 - Controls whether the render window should also contain a depth/stencil buffer. This is useful if you are not using intermediary render textures and are rendering directly to the render window. Most applications will set this to false and create the depth/stencil buffer manually as a render texture (see below).

@ref BansheeEngine::RENDER_WINDOW_DESC::gamma "RENDER_WINDOW_DESC::gamma"
 - Controls whether or not should pixels written to be render target be automatically gamma corrected (multiplied by inverse sRGB gamma). If your rendering calculations are in linear space and you aren't performing gamma correction manually you should set this to true.

An example that creates a 1280x720 window:
~~~~~~~~~~~~~{.cpp}
RENDER_WINDOW_DESC desc;
desc.videoMode = VideoMode(1280, 720);
desc.fullscreen = false;

SPtr<RenderWindow> window = RenderWindow::create(desc);
~~~~~~~~~~~~~

## Manipulating the window {#renderTargets_a_a}
Once the window is created you can perform the following actions:
 - @ref BansheeEngine::RenderWindow::resize "RenderWindow::resize"
 - @ref BansheeEngine::RenderWindow::move "RenderWindow::move"
 - @ref BansheeEngine::RenderWindow::setFullscreen "RenderWindow::setFullscreen"
 - @ref BansheeEngine::RenderWindow::setWindowed "RenderWindow::setWindowed"
 
And more. Read the @ref BansheeEngine::RenderWindow "RenderWindow" reference for the rest.

You can also use the window to convert between screen and window coordinates by calling:
 - @ref BansheeEngine::RenderWindow::screenToWindowPos "RenderWindow::screenToWindowPos"
 - @ref BansheeEngine::RenderWindow::windowToScreenPos "RenderWindow::windowToScreenPos"
 
# Render textures {#renderTargets_b}
Render textures are simpler than windows, but you are given more control over their format. Internally a render texture is just a @ref BansheeEngine::Texture "Texture" created with the @ref BansheeEngine::TU_RENDERTARGET "TU_RENDERTARGET" flag. Read the [texture](@ref textures) manual to learn how to create such textures.

Render texture must contain at least one texture (color surface), but may optionally also contain a depth-stencil surface. Depth-stencil surface is also a @ref BansheeEngine::Texture "Texture", but one created with the @ref BansheeEngine::TU_DEPTHSTENCIL "TU_DEPTHSTENCIL" flag. Depth stencil surfaces only accept specific pixel formats starting with "D" prefix in @ref BansheeEngine::PixelFormat "PixelFormat". Dimensions of the depth stencil surface must match the color surface.

To create a render texture call @ref BansheeEngine::RenderTexture::create(const RENDER_TEXTURE_DESC&) "RenderTexture::create" with a populated @ref BansheeEngine::RENDER_TEXTURE_DESC "RENDER_TEXTURE_DESC" structure. This structure expects a reference to one or more color surface textures, and an optional depth-stencil surface texture. For each of those you must also specify the face and mip level onto which to render, in case your texture has multiple.

For example:
~~~~~~~~~~~~~{.cpp}
// Create a 1280x720 texture with 32-bit RGBA format
TEXTURE_DESC colorDesc;
colorDesc.type = TEX_TYPE_2D;
colorDesc.width = 1280;
colorDesc.heigth = 720;
colorDesc.format = PF_R8G8B8A8;
colorDesc.usage = TU_RENDERTARGET;

HTexture color = Texture::create(colorDesc);

// Create a 1280x720 texture with a 32-bit depth-stencil format
TEXTURE_DESC depthDesc;
depthDesc.type = TEX_TYPE_2D;
depthDesc.width = 1280;
depthDesc.heigth = 720;
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

Optionally you can also use the overloaded @ref BansheeEngine::RenderTexture::create(const TEXTURE_DESC&, bool, PixelFormat) "RenderTexture::create" to create the texture surfaces and the render texture in one go:
~~~~~~~~~~~~~{.cpp}
// Has the same result as above:
TEXTURE_DESC colorDesc;
colorDesc.type = TEX_TYPE_2D;
colorDesc.width = 1280;
colorDesc.heigth = 720;
colorDesc.format = PF_R8G8B8A8;

SPtr<RenderTexture> renderTexture = RenderTexture::create(colorDesc, true, PF_D24S8);
~~~~~~~~~~~~~

## Multiple surfaces {#renderTargets_b_a}
Render textures can also contain multiple color surfaces (up to 8). Such targets simply allow you to write more data at once, improving performance as you do not need to execute multiple draw calls. To create a texture with multiple color surfaces simply fill out other entries of @ref BansheeEngine::RENDER_TEXTURE_DESC "RENDER_TEXTURE_DESC::colorSurfaces" array and proceed the same as in the above example.

Use `BS_MAX_MULTIPLE_RENDER_TARGETS` to learn what is the maximum supported number of color surfaces per target (usually 8). All color surfaces (and the depth/stencil surface) must have the same dimensions and sample count.

## Multi-sampled surfaces {#renderTargets_b_c}
Same as windows, render textures can be created with support for multiple samples per pixel. This allows affects such as multi-sampled antialiasing and similar. To create a multi-sampled render texture simply create a @ref BansheeEngine::Texture "Texture" with its `multisampleCount` parameter larger than one, which you then use to initialize a render texture. Make sure that all surfaces (including depth-stencil) in a render texture have the same number of samples.

In order to display a multi-sampled render texture on the screen you need to resolve the texture into a non-multi-sampled one. For windows this is done automatically for you, but for textures you must do it manually. You can do it from the GPU program (render API dependant), or by calling @ref BansheeEngine::TextureCore::copy "TextureCore::copy". 

@ref BansheeEngine::TextureCore::copy "TextureCore::copy" works only on the core thread, and it will copy contents of one texture to another texture. If you ensure that the source texture is multi-sampled but the destination is not, it will automatically resolve the multi-sampled texture.

# Priority {#renderTargets_c}
All render targets have a priority that can be set by calling @ref BansheeEngine::RenderTarget::setPriority "RenderTarget::setPriority". This priority can be used as a hint to the renderer in which order should the targets be rendered to. Targets with higher priority will be rendered to before targets with lower priority. If you are rendering manually using the @ref BansheeEngine::RenderAPI "RenderAPI" then this value is not used.

# Using render targets {#renderTargets_d}
Now that we know how to create a render target we can use it in one of various ways: render to it, read from it, clear it, or use its contents to render something else.

## Rendering to targets {#renderTargets_d_a}
Use @ref BansheeEngine::RenderAPI::setRenderTarget "RenderAPI::setRenderTarget" to bind a render target for rendering. After binding you may issue draw calls, after which you should follow it with a call to @ref BansheeEngine::RenderAPI::swapBuffers "RenderAPI::swapBuffers" if the target has multiple buffers (like a window). Read the [render API](@ref renderAPI) manual for more information.

## Clearing targets {#renderTargets_d_b}
Call @ref BansheeEngine::RenderAPI::clearRenderTarget "RenderAPI::clearRenderTarget" to clear all or portions of a render target to a specific color/value. Read the [render API](@ref renderAPI) manual for more information.

## Reading from targets {#renderTargets_d_c}
If you wish to read render target pixels from the CPU, you can use the normal @ref BansheeEngine::Texture "Texture" read functionality as described in the [texture](@ref textures) manual. To retrieve a @ref BansheeEngine::Texture "Texture" from a @ref BansheeEngine::RenderTexture "RenderTexture" call the following methods:
 - @ref BansheeEngine::RenderTexture::getColorTexture "RenderTexture::getColorTexture"
 - @ref BansheeEngine::RenderTexture::getDepthStencilTexture "RenderTexture::getDepthStencilTexture"
 
Before reading you must make sure the render target is not currently bound for rendering otherwise this will fail. You cannot read from a @ref BansheeEngine::RenderWindow "RenderWindow".

## Rendering using the targets {#renderTargets_c_d}
Sometimes you render to a render texture only to use it as input in another rendering operation. In such case the render texture is first used for writing, and then for reading. This case is similar to above, first you make sure that the render texture is not currently bound for writing, and then you call one of the above listed methods to retrieve the required @ref BansheeEngine::Texture "Texture".

Then you can bind the texture for rendering as normal (described in the [render API](@ref renderAPI) manual).

For depth/stencil surfaces there is a special case where you might need to leave them bound as a render target to be used for depth/stencil testing (not writing), but also for reading within a GPU program. In such a case you can call @ref BansheeEngine::RenderAPI::setRenderTarget "RenderAPI::setRenderTarget" with `readOnlyDepthStencil` parameter set to true.