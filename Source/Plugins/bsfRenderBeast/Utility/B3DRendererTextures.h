//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DRenderBeastPrerequisites.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Textures that get loaded on the main thread and get passed to the renderer. */
		struct LoadedRendererTextures
		{
			/** Default texture to use for Bokeh flare. */
			TShared<Texture> BokehFlare;
		};

		/** Contains static textures required for various render techniques. */
		class RendererTextures
		{
		public:
			/** Initializes the renderer textures. Must be called before using the textures. */
			static void StartUp(const LoadedRendererTextures& textures);

			/** Cleans up renderer textures. */
			static void ShutDown();

			/**
			 * 2D 2-channel texture containing a pre-integrated G and F factors of the microfactet BRDF. This is an
			 * approximation used for image based lighting, so we can avoid sampling environment maps for each light. Works in
			 * tandem with the importance sampled reflection cubemaps.
			 *
			 * (u, v) = (NoV, roughness)
			 * (r, g) = (scale, bias)
			 */
			static TShared<Texture> preintegratedEnvGF;

			/** Tileable 4x4 texture to be used for randomization in SSAO rendering. */
			static TShared<Texture> ssaoRandomization4x4;

			/** Cubemap containing indirect lighting, when no other is available. */
			static TShared<Texture> defaultIndirect;

			/** Texture used for coloring the lens flare effect depending on its distance from screen center. */
			static TShared<Texture> lensFlareGradient;

			/** Default texture to use for Bokeh flare. */
			static TShared<Texture> bokehFlare;

			/** Texture that controls which color channels to shift in the chromatic aberration effect. */
			static TShared<Texture> chromaticAberrationFringe;
		};

		/** @} */
	} // namespace render
} // namespace b3d
