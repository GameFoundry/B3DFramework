//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
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
	class BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget)) RenderTargetEx
	{
	public:
		/** @copydoc RenderTargetProperties::width */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget),n:Width,pr:getter)
		static u32 GetWidth(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::height */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget),n:Height,pr:getter)
		static u32 GetHeight(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::hwGamma */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget),n:GammaCorrection,pr:getter)
		static bool GetGammaCorrection(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::priority */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget),n:Priority,pr:getter)
		static i32 GetPriority(const SPtr<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::priority */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget),n:Priority,pr:setter)
		static void SetPriority(const SPtr<RenderTarget>& thisPtr, i32 priority);

		/** @copydoc RenderTargetProperties::multisampleCount */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget),n:SampleCount,pr:getter)
		static u32 GetSampleCount(const SPtr<RenderTarget>& thisPtr);
	};

	/** Extension class for RenderTexture, for adding additional functionality for the script interface. */
	class BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture)) RenderTextureEx
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
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static SPtr<RenderTexture> Create(PixelFormat format, int width, int height, int numSamples = 1, bool gammaCorrection = false,
			bool createDepth = false, PixelFormat depthStencilFormat = PF_D32);

		/**
		 * Creates a new 2D render texture using an existing color texture, and no depth-stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture to render color data to.
		 */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static SPtr<RenderTexture> Create(BS_NORREF const HTexture& colorSurface);

		/**
		 * Creates a new 2D render texture using existing textures as render destinations.
		 *
		 * @param[in]	colorSurface			Color texture to render color data to.
		 * @param[in]	depthStencilSurface		Optional depth/stencil texture to render depth/stencil data to.
		 */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static SPtr<RenderTexture> Create(BS_NORREF const HTexture& colorSurface,
			BS_NORREF const HTexture& depthStencilSurface);

		/**
		 * Creates a new 2D render texture using one or multiple color textures and no depth-stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture(s) to render color data to.
		 */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static SPtr<RenderTexture> Create(BS_NORREF const Vector<HTexture>& colorSurface);

		/**
		 * Creates a new 2D render texture using one or multiple color textures and a depth/stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture(s) to render color data to.
		 * @param[in]	depthStencilSurface		Optional depth/stencil texture to render depth/stencil data to.
		 */
		BS_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static SPtr<RenderTexture> Create(BS_NORREF const Vector<HTexture>& colorSurface,
			BS_NORREF const HTexture& depthStencilSurface);

		/** Returns the primary color surface that contains rendered color data. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture),n:ColorSurface,pr:getter)
		static BS_NORREF HTexture GetColorSurface(const SPtr<RenderTexture>& thisPtr);

		/** Returns all the color surfaces. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture),n:ColorSurfaces,pr:getter)
		static BS_NORREF Vector<HTexture> GetColorSurfaces(const SPtr<RenderTexture>& thisPtr);

		/** Returns the depth/stencil surface that contains rendered depth and stencil data. */
		BS_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture),n:DepthStencilSurface,pr:getter)
		static BS_NORREF HTexture GetDepthStencilSurface(const SPtr<RenderTexture>& thisPtr);
	};

	/** @endcond */
	/** @} */
}
