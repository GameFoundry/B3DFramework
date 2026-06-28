//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

#define B3D_VERIFY_BARRIERS B3D_BUILD_TYPE_DEVELOPMENT // If enabled, ensures that memory barriers are properly issued

namespace b3d
{
	namespace render
	{
		/** @addtogroup GpuBackend
		 *  @{
		 */

		/** Keeps track on which pipeline stages a resource was written/read, and on which stages it may be safely accessed from. */
		struct GpuWriteHazardPipelineTracking
		{
			static constexpr u32 kPipelineStageCount = 16;

			/** For each pipeline stage, stores in which stages is it safe to access the resource. */
			std::array<GpuStageFlags, kPipelineStageCount> SafeAccess;

			GpuWriteHazardPipelineTracking();

			/** Clears safe access for all provided pipeline stages. */
			void ClearStageSafeAccess(GpuStageFlags stages);

			/**
			 * Adds safe access for all provided pipeline stages.
			 *
			 * @param	sourceStages		One or multiple stages to add the safe access to.
			 * @param	destinationStages	Stages to register as being safe to access from.
			 */
			void AddStageSafeAccess(GpuStageFlags sourceStages, GpuStageFlags destinationStages);

			/** Checks is it safe to access the resource in all the provided pipeline stages. */
			bool IsAccessSafe(GpuStageFlags stages) const;

			/** Returns a list of all source stages that we cannot safely access data from the provided @p stages. */
			GpuStageFlags GetUnsafeAccessStages(GpuStageFlags stages) const;

			/** Writes a descriptive error message when access is unsafe. */
			void LogUnsafeAccess(GpuStageFlags stages, GpuAccessFlags currentAccessType, GpuAccessFlags previousAccessType) const;
		};

		/** Tracking that is used for validation when memory barriers need to be issued. */
		struct GpuWriteHazardTracking
		{
			GpuAccessFlags Access; /**< Has the buffer been read or written so far. */

			/**
			 * Keeps track of all pipeline stages that the resource was read from, and which of those stages can be safely accessed by a write operation (and on which stage).
			 * A write following a read requires an execution barrier.
			 */
			GpuWriteHazardPipelineTracking ExecutionBarrierTracking;

			/**
			 * Keeps track of all pipeline stages that the resource was written to, and which of those stages can be safely accessed by a read or write operation
			 * (and on which stage). Any operation following a write requires a memory barrier. Memory barrier implies an execution barrier as well.
			 */
			GpuWriteHazardPipelineTracking MemoryBarrierTracking;

			/** Updates execution and memory barrier tracking by marking access as safe in the destination stages, from the source stages. */
			void AddSafeAccess(GpuStageFlags sourceAccessStageFlags, GpuAccessFlags sourceAccess, GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess);

#if B3D_VERIFY_BARRIERS
			/** Verifies that the access is safe from the provided stage and access type. Logs errors if not. */
			void VerifySafeAccess(GpuStageFlags destinationAccessStageFlags, GpuAccessFlags destinationAccess) const;
#endif

			// Note - Add LayoutTransitionTracking? Tracks last use of an imagine in a specific layout, and which layouts can be safely transitioned to/from without a barrier
		};

		/** @} */
	} // namespace render
} // namespace b3d
