//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class MetalBarrierHelper;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<MetalBarrierHelper>;

	/** Metal-specific resource tracker. Inherits the backend-agnostic tracking machinery from TGpuResourceTracker. */
	class MetalResourceTracker : public TGpuResourceTracker<MetalBarrierHelper>
	{ };

	/** @} */
} // namespace b3d::render
