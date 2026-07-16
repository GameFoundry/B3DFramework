//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DRenderTexture.h"

namespace b3d
{
	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	/**
	 * Metal implementation of a render texture.
	 *
	 * @note	Main thread only.
	 */
	class MetalRenderTexture : public RenderTexture
	{
	public:
		MetalRenderTexture(const RenderTextureCreateInformation& createInformation);
		virtual ~MetalRenderTexture() = default;
	};

	namespace render
	{
		/**
		 * Metal implementation of a render texture.
		 *
		 * Unlike the Vulkan render proxy, this deliberately does not override Initialize() to build a framebuffer
		 * object: Metal has no VkFramebuffer/VkRenderPass equivalent. MTLRenderPassDescriptor is a transient,
		 * cheap CPU-side descriptor whose load/store actions are per-pass state, so the command buffer constructs
		 * it on every BeginRenderPass directly from the surface bindings exposed below (face/mip selection maps to
		 * the attachment's slice/level). The core RenderTexture::Initialize() still runs and performs view
		 * creation and validation.
		 *
		 * @note	Render thread only.
		 */
		class MetalRenderTexture : public RenderTexture
		{
		public:
			MetalRenderTexture(const RenderTextureCreateInformation& createInformation);
			~MetalRenderTexture() override = default;

			/** Returns the full color surface binding (texture + face + mip + face count) at the given attachment index. */
			const RenderSurfaceInformation& GetColorSurface(u32 attachmentIndex) const { return mInformation.ColorSurfaces[attachmentIndex]; }

			/** Returns the full depth/stencil surface binding, or one with a null Texture if none is attached. */
			const RenderSurfaceInformation& GetDepthStencilSurface() const { return mInformation.DepthStencilSurface; }
		};

	} // namespace render

	/** @} */
} // namespace b3d
