//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsRenderer.h"
#include "BsRenderQueue.h"

namespace bs { namespace ct
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
		RenderBeastOptions() { }

		/**	Type of filtering to use for all textures on scene elements. */
		RenderBeastFiltering filtering = RenderBeastFiltering::Anisotropic;

		/**
		 * Maximum number of samples to be used when performing anisotropic filtering. Only relevant if #filtering is set to
		 * RenderBeastFiltering::Anisotropic.
		 */
		UINT32 anisotropyMax = 16;

		/**
		 * Controls if and how a render queue groups renderable objects by material in order to reduce number of state
		 * changes. Sorting by material can reduce CPU usage but could increase overdraw.
		 */
		StateReduction stateReductionMode = StateReduction::Distance;
	};

	/** @} */
}}