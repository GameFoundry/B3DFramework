//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "GpuBackend/B3DRenderTexture.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Extension class for RenderTarget, for adding additional functionality for the script interface. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(RenderTarget)) RenderTargetEx
	{
	public:
		/** @copydoc RenderTargetProperties::Width */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget), ExportName(Width), Property(Getter))
		static u32 GetWidth(const TShared<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::Height */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget), ExportName(Height), Property(Getter))
		static u32 GetHeight(const TShared<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::HwGamma */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget), ExportName(GammaCorrection), Property(Getter))
		static bool GetGammaCorrection(const TShared<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::Priority */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget), ExportName(Priority), Property(Getter))
		static i32 GetPriority(const TShared<RenderTarget>& thisPtr);

		/** @copydoc RenderTargetProperties::Priority */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget), ExportName(Priority), Property(Setter))
		static void SetPriority(const TShared<RenderTarget>& thisPtr, i32 priority);

		/** @copydoc RenderTargetProperties::MultisampleCount */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTarget), ExportName(SampleCount), Property(Getter))
		static u32 GetSampleCount(const TShared<RenderTarget>& thisPtr);
	};

	/** Extension class for RenderTexture, for adding additional functionality for the script interface. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(RenderTexture)) RenderTextureEx
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
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static TShared<RenderTexture> Create(PixelFormat format, int width, int height, int numSamples = 1, bool gammaCorrection = false, bool createDepth = false, PixelFormat depthStencilFormat = PF_D32);

		/**
		 * Creates a new 2D render texture using an existing color texture, and no depth-stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture to render color data to.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static TShared<RenderTexture> Create(B3D_NO_RREF const HTexture& colorSurface);

		/**
		 * Creates a new 2D render texture using existing textures as render destinations.
		 *
		 * @param[in]	colorSurface			Color texture to render color data to.
		 * @param[in]	depthStencilSurface		Optional depth/stencil texture to render depth/stencil data to.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static TShared<RenderTexture> Create(B3D_NO_RREF const HTexture& colorSurface, B3D_NO_RREF const HTexture& depthStencilSurface);

		/**
		 * Creates a new 2D render texture using one or multiple color textures and no depth-stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture(s) to render color data to.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static TShared<RenderTexture> Create(B3D_NO_RREF const Vector<HTexture>& colorSurface);

		/**
		 * Creates a new 2D render texture using one or multiple color textures and a depth/stencil texture.
		 *
		 * @param[in]	colorSurface			Color texture(s) to render color data to.
		 * @param[in]	depthStencilSurface		Optional depth/stencil texture to render depth/stencil data to.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(RenderTexture))
		static TShared<RenderTexture> Create(B3D_NO_RREF const Vector<HTexture>& colorSurface, B3D_NO_RREF const HTexture& depthStencilSurface);

		/** Returns the primary color surface that contains rendered color data. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture), ExportName(ColorSurface), Property(Getter))
		static B3D_NO_RREF HTexture GetColorSurface(const TShared<RenderTexture>& thisPtr);

		/** Returns all the color surfaces. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture), ExportName(ColorSurfaces), Property(Getter))
		static B3D_NO_RREF Vector<HTexture> GetColorSurfaces(const TShared<RenderTexture>& thisPtr);

		/** Returns the depth/stencil surface that contains rendered depth and stencil data. */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(RenderTexture), ExportName(DepthStencilSurface), Property(Getter))
		static B3D_NO_RREF HTexture GetDepthStencilSurface(const TShared<RenderTexture>& thisPtr);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
