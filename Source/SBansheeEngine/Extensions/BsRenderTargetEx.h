//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "RenderAPI/BsRenderTexture.h"

namespace bs
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	 /** Extension class for RenderTarget, for adding additional functionality for the script interface. */
	class BS_SCRIPT_EXPORT(e:RenderTarget) RenderTargetEx
	{
	public:
		/** @copydoc RenderTargetProperties::width */
		BS_SCRIPT_EXPORT(e:RenderTarget,n:Width,pr:getter)
		static UINT32 getWidth(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::height */
		BS_SCRIPT_EXPORT(e:RenderTarget,n:Height,pr:getter)
		static UINT32 getHeight(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::hwGamma */
		BS_SCRIPT_EXPORT(e:RenderTarget,n:GammaCorrection,pr:getter)
		static bool getGammaCorrection(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::priority */
		BS_SCRIPT_EXPORT(e:RenderTarget,n:Priority,pr:getter)
		static INT32 getPriority(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::priority */
		BS_SCRIPT_EXPORT(e:RenderTarget,n:Priority,pr:setter)
		static void setPriority(const SPtr<RenderTarget>& thisPtr, INT32 priority);

		/** @copydoc RenderTargetProperties::multisampleCount */
		BS_SCRIPT_EXPORT(e:RenderTarget,n:SampleCount,pr:getter)
		static UINT32 getSampleCount(const SPtr<RenderTarget>& thisPtr);
	};

	/** Extension class for RenderTexture, for adding additional functionality for the script interface. */
	class BS_SCRIPT_EXPORT(e:RenderTexture) RenderTextureEx
	{
	public:
		/**
		 * Creates a new 2D render texture.
		 *
		 * @param[in]	format				Pixel format of the texture. Format must be a valid uncompressed color format.
		 * @param[in]	width				Width of the texture in pixels.
		 * @param[in]	height				Height of the texture in pixels.
		 * @param[in]	numSamples			Number of samples contained per pixel.
		 * @param[in]	gammaCorrection		Determines should the pixels written on the texture be gamma corrected.
		 * @param[in]	createDepth			Should the render texture also contain a depth/stencil buffer.
		 * @param[in]	depthStencilFormat	Format of the depth/stencil buffer, if @p createDepth is enabled. Format must
		 *									be a valid depth/stencil format.
		 */
		BS_SCRIPT_EXPORT(ec:RenderTexture)
		static SPtr<RenderTexture> create(PixelFormat format, int width, int height, int numSamples = 1, bool gammaCorrection = false,
			bool createDepth = false, PixelFormat depthStencilFormat = PF_D32);

		/**
		 * Creates a new 2D render texture using an existing color texture, and no depth-stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture to render color data to.
		 */
		BS_SCRIPT_EXPORT(ec:RenderTexture)
		static SPtr<RenderTexture> create(const HTexture& colorSurface);

		/**
		 * Creates a new 2D render texture using existing textures as render destinations.
		 *
		 * @param[in]	colorSurface			Color texture to render color data to.
		 * @param[in]	depthStencilSurface		Optional depth/stencil texture to render depth/stencil data to.
		 */
		BS_SCRIPT_EXPORT(ec:RenderTexture)
		static SPtr<RenderTexture> create(const HTexture& colorSurface, const HTexture& depthStencilSurface);

		/**
		 * Creates a new 2D render texture using one or multiple color textures and no depth-stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture(s) to render color data to.
		 */
		BS_SCRIPT_EXPORT(ec:RenderTexture)
		static SPtr<RenderTexture> create(const Vector<HTexture>& colorSurface);

		/**
		 * Creates a new 2D render texture using one or multiple color textures and a depth/stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture(s) to render color data to.
		 * @param[in]	depthStencilSurface		Optional depth/stencil texture to render depth/stencil data to.
		 */
		BS_SCRIPT_EXPORT(ec:RenderTexture)
		static SPtr<RenderTexture> create(const Vector<HTexture>& colorSurface, const HTexture& depthStencilSurface);

		/** Returns the primary color surface that contains rendered color data. */
		BS_SCRIPT_EXPORT(e:RenderTexture,n:ColorSurface,pr:getter)
		static HTexture getColorSurface(const SPtr<RenderTexture>& thisPtr);

		/** Returns all the color surfaces. */
		BS_SCRIPT_EXPORT(e:RenderTexture,n:ColorSurfaces,pr:getter)
		static Vector<HTexture> getColorSurfaces(const SPtr<RenderTexture>& thisPtr);

		/** Returns the depth/stencil surface that contains rendered depth and stencil data. */
		BS_SCRIPT_EXPORT(e:RenderTexture,n:DepthStencilSurface,pr:getter)
		static HTexture getDepthStencilSurface(const SPtr<RenderTexture>& thisPtr);
	};

	/** @endcond */
	/** @} */
}
