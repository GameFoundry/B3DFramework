//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "GpuBackend/B3DGpuHazards.h"

namespace b3d::render
{
	/**
	 * Can be implemented by specific backends to store perform additional tracking for GPU images. Allows backends to 
	 * extend the functionality already provided by TGpuResourceTracker.
	 */
	class GpuImageNativeState
	{
	public:
		GpuImageNativeState() = default;
		virtual ~GpuImageNativeState() = default;

		/** Copies state. */
		virtual TShared<GpuImageNativeState> Clone() const = 0;

		/** 
		 * Determines if native transition is required for an upcoming access. If a transition already exists, merges it with an existing transition. 
		 * Returns true the first time a transition is required (i.e. not on merges, or when no transition is required). 
		 */
		virtual bool AccumulateTransitionRequirement(GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access) = 0;

		/** Records an actual access after its preceding native transitions and barriers. */
		virtual void RecordAccess(GpuStageFlags stages, GpuAccessFlags access) = 0;

		/** Updates hazard tracking for a recorded barrier. Only writes preceding the barrier gain visibility. */
		virtual void RecordBarrier(const GpuBarrierScope& barrier) = 0;

		/**
		 * Determines necessary transitions at command buffer submission time, based on the state of the resource
		 * is the preceeding command buffer. Updated subresource with new native submission state. Returns
		 * hazard state for the resources, with hazards for any transitions that had occurred.
		 *
		 * Submit thread only.
		 */
		virtual const GpuResourceHazardState& ResolveSubmission(IGpuResource& subresource) = 0;
	};
}
