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
		struct B3D_EXPORT GpuWriteHazardPipelineTracking
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
		struct B3D_EXPORT GpuWriteHazardTracking
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

		/** Contains information about a single resource bound/used on a command buffer. */
		struct GpuResourceUseHandle
		{
			/** Whether this resource has been submitted to the GPU queue. */
			bool Used;

			/** Access flags indicating how the resource is being accessed (read/write). */
			GpuAccessFlags Flags;
		};

		/** Contains information about a single GPU buffer resource bound/used on a command buffer. */
		struct GpuBufferTrackingState
		{
			/** Information about resource usage and submission state. */
			GpuResourceUseHandle UseHandle;

			/** Flags indicating how the buffer is being used (shader access, transfer, etc.). */
			GpuResourceUseFlags UseFlags;

			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued. */
			GpuWriteHazardTracking* WriteHazardTracking = nullptr;

#if B3D_BUILD_TYPE_DEVELOPMENT
			/** Suballocation indices that are bound in this tracking state. Typically 1-2. */
			TInlineArray<u32, 2> BoundSuballocationIndices;
#endif
		};

		/** Contains information about a single GPU image resource bound/used on a command buffer. */
		struct GpuImageTrackingState
		{
			/** Information about resource usage and submission state. */
			GpuResourceUseHandle UseHandle;

			/** Index of the first subresource tracking state in the global subresource tracking array. */
			u32 FirstSubresourceInfoIndex;

			/** Number of consecutive subresource tracking states belonging to this image. */
			u32 SubresourceInfoCount;
		};

		/** Contains information about a range of GPU image sub-resources bound/used on a command buffer. */
		struct GpuImageSubresourceTrackingState
		{
			/** The subresource range (mip levels and array layers) covered by this tracking state. */
			GpuTextureSubresourceRange Range;

			// Storing stage & access flags separately per use category so they can be cleared independantly when that use
			// ends (e.g. image unbound as FB attachment, or memory barrier executed)

			/** Use flags when subresource is bound for shader reads or writes. Reset after resource is unbound. */
			GpuAccessFlags ShaderUse;

			/** Use flags when subresource is bound as a framebuffer attachment. Reset after resource is unbound. */
			GpuAccessFlags FramebufferUse;

			/** Specifies how will the subresource be accessed during the current render pass or dispatch call. Unlike accesses in *Use structs, this one is not reset after render pass. */
			GpuAccessFlags Access;

			/** Determines is the initial use of this subresource read-only. Used for better determining access flags. */
			bool InitialReadOnly = false;

			/** Used for tracking read-after-write/write-after-write and write-after-read hazards, and validating that correct barriers were issued. */
			GpuWriteHazardTracking* WriteHazardTracking = nullptr;

			// Only relevant for layout transitions
			/**
			 * Layout transition performed during the command buffer submit. This will be the initial layout of the
			 * image when the command buffer starts executing.
			 */
			GpuImageLayout InitialLayout;

			/**
			 * Layout the image is currently in. This will be the initial layout if no other transition was performed, or
			 * layout resulting from the last performed transition.
			 */
			GpuImageLayout CurrentLayout;

			/**
			 * Stores the layout that the image needs to be before being used in the current render pass or dispatch call.
			 * Equal to CurrentLayout if no transition is needed. Updated after every render pass or dispatch call.
			 */
			GpuImageLayout RequiredLayout;

			/**
			 * Layout the image will have after the render pass executes, taking account automatic transitions render pass
			 * does on its attachments. Only relevant for framebuffer attachments. Ignored if render pass doesn't execute.
			 */
			GpuImageLayout RenderPassLayout;
		};

		/** @} */
	} // namespace render
} // namespace b3d
