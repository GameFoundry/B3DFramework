//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"
#include "GpuBackend/B3DGpuResourceTracker.h"

namespace b3d::render
{
	class MetalBarrierHelper;
	class MetalResourceTracker;

	/** @addtogroup MetalGpuBackend
	 *  @{
	 */

	extern template class TGpuResourceTracker<MetalResourceTracker, MetalBarrierHelper>;

	/** Metal-specific resource tracker. Inherits the backend-agnostic tracking machinery from TGpuResourceTracker. */
	class MetalResourceTracker : public TGpuResourceTracker<MetalResourceTracker, MetalBarrierHelper>
	{
	public:
		/**
		 * Encoding of GpuImageLayout::ShaderReadOnly in GpuImageNativeState::Layout, for images that can rest (see TGpuResourceTracker).
		 * Metal has no layouts, so the native layout keeps its initial value.
		 */
		static constexpr u32 kRestingNativeLayout = (u32)GpuImageLayout::Undefined;

		/** Metal has no layouts, so layout transitions never touch the image (see GpuSubmissionTransition::Build()). */
		static constexpr bool kLayoutTransitionsAreWrites = false;
	};

	/** @} */
} // namespace b3d::render
