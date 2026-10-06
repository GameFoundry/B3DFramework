//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

#if B3D_GPU_EXPLICIT_BARRIERS && B3D_BUILD_TYPE_DEVELOPMENT

#include "Utility/B3DDenseMap.h"

namespace b3d
{
	class IGpuBufferResource;
	class IGpuImageResource;
}

namespace b3d::render
{
	/** @addtogroup GpuBackend-Internal
	 *  @{
	 */

	/**
	 * Development-only shadow of the resource states that explicit barriers declare on one command buffer. Checks the source of each
	 * explicit barrier against the destination of the previous barrier and the accesses since, and checks each access against the
	 * destination of the last barrier. Accesses before the first barrier of a resource on the command buffer are not checked, because
	 * an earlier command buffer declared their state, but the source of that first barrier is checked against them.
	 *
	 * Stages are engine stages, so the checks give the same result on every backend. Accesses report the engine stages of their
	 * operation (GpuStageFlag::Transfer for a copy), not the stages a backend runs the operation in.
	 */
	class B3D_EXPORT GpuExplicitBarrierValidator
	{
	public:
		/**
		 * Checks an explicit barrier of a buffer, and declares its destination.
		 *
		 * @param	buffer				Buffer the barrier applies to.
		 * @param	barrier				Barrier to check.
		 * @param	phase				Part of the barrier the command buffer records.
		 * @param	split				Split barrier connecting the halves. Null for GpuBarrierPhase::Full.
		 */
		void ValidateBarrier(IGpuBufferResource* buffer, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split);

		/** Checks an explicit barrier of a range of image subresources, and declares its destination. See the buffer overload. */
		void ValidateBarrier(IGpuImageResource* image, const GpuTextureSubresourceRange& range, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split);

		/** Checks an access of a buffer in @p stages against the declared state, and records it for the next barrier. */
		void ValidateAccess(IGpuBufferResource* buffer, GpuStageFlags stages, GpuAccessFlags access);

		/**
		 * Checks an access of a range of image subresources in @p stages against the declared state, and records it for the
		 * next barrier. @p layout is only checked if it is not GpuImageLayout::Undefined, which accesses that do not depend on the
		 * layout pass.
		 */
		void ValidateAccess(IGpuImageResource* image, const GpuTextureSubresourceRange& range, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/** Forgets every state. Call when the command buffer is reset. */
		void Clear();

	private:
		/** State of a buffer or an image subresource on the command buffer. */
		struct ResourceState
		{
			GpuAccessState Declared; /**< Destination of the last barrier. Valid if IsDeclared is true. */
			GpuStageFlags UsedStages; /**< Stages that accessed the resource since the last barrier. */
			GpuAccessFlags UsedAccess; /**< How the resource was accessed since the last barrier. */
			const GpuSplitBarrier* PendingAcquire = nullptr; /**< Split barrier this command buffer released, but did not acquire yet. */
			bool IsDeclared = false;
		};

		/** Checks @p barrier against @p state and applies it. Returns an error message, or null if the barrier is valid. */
		static const char* ApplyBarrier(ResourceState& state, const GpuExplicitBarrier& barrier, GpuBarrierPhase phase, const GpuSplitBarrier* split, bool isImage);

		/** Checks an access against @p state and records it. Returns an error message, or null if the access is valid. */
		static const char* ApplyAccess(ResourceState& state, GpuImageLayout layout, GpuStageFlags stages, GpuAccessFlags access);

		/** Calls @p function with the state of each subresource of @p image in @p range. Returns the first error message it returns. */
		template<class TFunction>
		const char* ForEachSubresourceState(IGpuImageResource* image, const GpuTextureSubresourceRange& range, TFunction&& function);

		TDenseMap<IGpuBufferResource*, ResourceState> mBuffers;
		TDenseMap<IGpuImageResource*, Vector<ResourceState>> mImages; /**< One state per face, mip level and aspect. */
	};

	/** @} */
} // namespace b3d::render

#endif
