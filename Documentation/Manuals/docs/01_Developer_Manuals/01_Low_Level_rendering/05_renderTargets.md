---
title: Render targets
---

Before we can actually render an object, we must also specify a render target which will serve as a surface to render the output to. We already covered render target creation in [windows](User_Manuals/Advanced_Rendering/windows) and [offscreen rendering](User_Manuals/Advanced_Rendering/offscreenRendering) manuals. The only difference is that in those manuals we used a **Camera** component to set the target, and relied on the renderer to make use of it.

In this chapter we'll show how to manually bind a render target for rendering, as well as some other render target related operations.

# Binding
Use @b3d::render::GpuCommandBuffer::BeginRenderPass to bind a render target for rendering. This begins a render pass, which must be closed with @b3d::render::GpuCommandBuffer::EndRenderPass.

~~~~~~~~~~~~~{.cpp}
TShared<RenderTarget> target = ...; // Create a RenderTexture or RenderWindow as described in earlier chapters
TShared<GpuCommandBuffer> commandBuffer = ...; // Obtain a command buffer from a command buffer pool

RenderPassCreateInformation renderPassInfo(target);
commandBuffer->BeginRenderPass(renderPassInfo);

// ... perform rendering operations ...

commandBuffer->EndRenderPass();
~~~~~~~~~~~~~

This will bind the entirety of the render target surface for rendering. If you just want to render to a portion of the target you can also call @b3d::render::GpuCommandBuffer::SetViewport. It accepts a 2D area whose coordinates should be in [0, 1] range and it specifies in which portion of the render target should rendering occur.

~~~~~~~~~~~~~{.cpp}
// Render to the center of the render target, at 50% of its size
commandBuffer->SetViewport(Area2(0.25f, 0.25f, 0.5f, 0.5f));
~~~~~~~~~~~~~

## Advanced binding
**render::GpuCommandBuffer::BeginRenderPass()** accepts a @b3d::RenderPassCreateInformation structure with parameters to control more advanced behaviour:
 - `ReadOnlyMask` - Combination of one or more elements of @b3d::RenderSurfaceMaskBits denoting which surfaces will be bound for read-only operations. This is useful for depth or stencil surfaces which might need to be bound both for depth/stencil tests, as well as shader reads. If you don't specify this the render backend will assume you will be writing to the render target which will result in undefined behaviour if you also try reading from that same texture.
 - `LoadMask` - Mask described by @b3d::RenderSurfaceMaskBits which controls if current contents of any of the render target surfaces should be preserved. By default the system doesn't guarantee the contents will be preserved and data is instead undefined. In certain cases (like blending operations) you want to preserve the contents, in which case specify the necessary flags to tell the system which surfaces need their contents preserved.

~~~~~~~~~~~~~{.cpp}
// Bind a render target with read-only depth/stencil, and preserve the existing contents of depth-stencil buffer on bind
RenderPassCreateInformation renderPassInfo(target, RT_DEPTH_STENCIL, RT_DEPTH_STENCIL);
commandBuffer->BeginRenderPass(renderPassInfo);
~~~~~~~~~~~~~

# Clearing
Usually a render target will be re-used many times. Unless you are sure that every use will completely overwrite the render target contents, it can be beneficial (and in some cases necessary) to clear the render target to some value. Call @b3d::render::GpuCommandBuffer::ClearRenderTarget to clear the currently bound render target. Its only parameter is a @b3d::RenderSurfaceMaskBits of which surfaces of the target to clear.

A surface is always cleared to the value it was created with. Choose that value through @b3d::TextureInformation::ClearColor, @b3d::TextureInformation::ClearDepth and @b3d::TextureInformation::ClearStencil when creating the texture (or the equivalent fields on @b3d::RenderWindowCreateInformation for a window), and it stays fixed for the lifetime of the resource. Clear commands cannot override it.

~~~~~~~~~~~~~{.cpp}
// The clear values are chosen when the surface textures are created
TextureCreateInformation colorTextureCreateInformation;
colorTextureCreateInformation.Width = 1280;
colorTextureCreateInformation.Height = 720;
colorTextureCreateInformation.Format = PF_RGBA8;
colorTextureCreateInformation.Usage = TextureUsageFlag::RenderTarget;
colorTextureCreateInformation.ClearColor = Color::kBlue;

// ... create the render texture from that texture, then bind it ...

// Clear color and depth surfaces. Color is cleared to blue, depth to the depth texture's own clear value
commandBuffer->ClearRenderTarget(RT_COLOR_ALL | RT_DEPTH);
~~~~~~~~~~~~~

You can also call @b3d::render::GpuCommandBuffer::ClearViewport to clear only the viewport portion of the render target. The parameters are identical to **render::GpuCommandBuffer::ClearRenderTarget()**.

Clearing the depth buffer is especially important as the GPU will read its contents during rendering, and having old data in the depth buffer pretty much guarantees your rendering will not be valid.

# Swapping
If a render target is a **RenderWindow** you must present the back buffer after rendering. This is because windows are usually double or triple buffered, meaning the rendering happens to a hidden buffer invisible to the user. When presenting is called this hidden buffer is presented to the user.

You can present a render window by calling @b3d::GpuDevice::PresentRenderWindow or @b3d::GpuQueue::PresentRenderWindow on a specific queue. The GPU device can be accessed through the command buffer via @b3d::render::GpuCommandBuffer::GetGpuDevice.

~~~~~~~~~~~~~{.cpp}
TShared<RenderWindow> window = ...; // Create a render window
TShared<GpuCommandBuffer> commandBuffer = ...; // Obtain a command buffer

RenderPassCreateInformation renderPassInfo(window);
commandBuffer->BeginRenderPass(renderPassInfo);

// ... draw something ...

commandBuffer->EndRenderPass();
commandBuffer->End();

// Submit the command buffer to the GPU
GpuWorkContext& gpuContext = render::GetRenderer()->GetGpuContext();
gpuContext.SubmitCommandBuffer(commandBuffer);

// Present the window's back buffer
GpuDevice& gpuDevice = commandBuffer->GetGpuDevice();
gpuDevice.PresentRenderWindow(window);
~~~~~~~~~~~~~

# Capturing window contents
A window's back buffer can only be read while it is being presented. Call @b3d::render::RenderWindow::RequestCapture to capture the next image presented to the window. The capture is read when the window is presented, after everything rendered to it that frame, and the provided async operation completes with the image once the GPU finishes the copy. A request stays pending until the window is presented with a rendered image. Call @b3d::render::RenderWindow::CancelCaptures to complete pending requests with a null image. Both methods must be called on the render thread.

~~~~~~~~~~~~~{.cpp}
TAsyncOp<TShared<PixelData>> captureOp;
window->RequestCapture(captureOp);

// ... render to the window and present it ...

// Once the GPU has finished the copy
TShared<PixelData> image = captureOp.GetReturnValue();
~~~~~~~~~~~~~

On Vulkan and Metal, reading a window that is not headless requires a development build, because only development builds create swap chain images that can be copied from.
