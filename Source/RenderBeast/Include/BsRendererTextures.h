//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	/** Contains static textures required for various render techniques. */
	class RendererTextures
	{
	public:
		/** Initializes the renderer textures. Must be called before using the textures. */
		static void startUp();

		/** Cleans up renderer textures. */
		static void shutDown();

		/**
		 * 2D 2-channel texture containing a pre-integrated G and F factors of the microfactet BRDF. This is an
		 * approximation used for image based lighting, so we can avoid sampling environment maps for each light. Works in
		 * tandem with the importance sampled reflection cubemaps.
		 * 
		 * (u, v) = (NoV, roughness)
		 * (r, g) = (scale, bias)
		 */
		static SPtr<Texture> preintegratedEnvGF;

		/** Tileable 4x4 texture to be used for randomization in SSAO rendering. */
		static SPtr<Texture> ssaoRandomization4x4;
	};

	/** @} */
}}