//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "Renderer/BsRenderer.h"
#include "Renderer/BsRenderQueue.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderBeast
		 *  @{
		 */

		/** Texture filtering options for RenderBeast. */
		enum class RenderBeastFiltering
		{
			Bilinear, /**< Sample linearly in X and Y directions within a texture mip level. */
			Trilinear, /**< Sample bilinearly and also between texture mip levels to hide the mip transitions. */
			Anisotropic /**< High quality dynamic filtering that improves quality of angled surfaces */
		};

		/** A set of options used for controlling the rendering of the RenderBeast renderer. */
		struct RenderBeastOptions : public RendererOptions
		{
			/**	Type of filtering to use for all textures on scene elements. */
			RenderBeastFiltering Filtering = RenderBeastFiltering::Anisotropic;

			/**
			 * Maximum number of samples to be used when performing anisotropic filtering. Only relevant if #filtering is set to
			 * RenderBeastFiltering::Anisotropic.
			 */
			u32 AnisotropyMax = 16;

			/**
			 * Controls if and how a render queue groups renderable objects by material in order to reduce number of state
			 * changes. Sorting by material can reduce CPU usage but could increase overdraw.
			 */
			StateReduction StateReductionMode = StateReduction::Distance;

			/**
			 * Determines the maximum shadow map size, in pixels. The system might decide to use smaller resolution maps for
			 * shadows far away, but will never increase the resolution past the provided value.
			 */
			u32 ShadowMapSize = 2048;
		};

		/** @} */
	} // namespace ct
} // namespace bs
