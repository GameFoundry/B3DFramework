//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsRenderBeastPrerequisites.h"
#include "BsModule.h"
#include "BsMatrix4.h"
#include "BsConvexVolume.h"

namespace bs { namespace ct
{
	/** @addtogroup RenderBeast
	 *  @{
	 */

	// TODO - Define normal and omni vertex shaders and their params
	// TODO - Move renderable objects from RenderBeast into a separate object so I can pass them here?
	//  - SceneInfo?

	/** Provides functionality for rendering shadow maps. */
	class ShadowRendering : public Module<ShadowRendering>
	{
	public:
		/**
		 * Generates a frustum for a single cascade of a cascaded shadow map. Also outputs spherical bounds of the
		 * split view frustum.
		 * 
		 * @param[in]	view		View whose frustum to split.
		 * @param[in]	lightDir	Direction of the light for which we're generating the shadow map.
		 * @param[in]	cascade		Index of the cascade to generate the frustum for.
		 * @param[in]	numCascades	Maximum number of cascades in the cascaded shadow map. Must be greater than zero.
		 * @param[out]	outBounds	Spherical bounds of the split view frustum.
		 * @return					Convex volume covering the area of the split view frustum visible from the light.
		 */
		static ConvexVolume getCSMSplitFrustum(const RendererView& view, const Vector3& lightDir, UINT32 cascade, 
			UINT32 numCascades, Sphere& outBounds);

		/**
		 * Finds the distance (along the view direction) of the frustum split for the specified index. Used for cascaded
		 * shadow maps.
		 * 
		 * @param[in]	view			View whose frustum to split.
		 * @param[in]	index			Index of the split. 0 = near plane.
		 * @param[in]	numCascades		Maximum number of cascades in the cascaded shadow map. Must be greater than zero
		 *								and greater or equal to @p index.
		 * @return						Distance to the split position along the view direction.
		 */
		static float getCSMSplitDistance(const RendererView& view, UINT32 index, UINT32 numCascades);
	};

	/* @} */
}}
