//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuTimelineFence.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/**
		 * Metal implementation of GpuTimelineFence.
		 *
		 * Wraps a single MTLSharedEvent. The fence's monotonically increasing 64-bit value is the
		 * event's signaledValue, advanced on the GPU side by encodeSignalEvent:value: encoded into
		 * the command buffer at submit time (see MetalGpuCommandBuffer::CommitInternal). CPU readers
		 * poll @c GetCompletedValue, which is lock-free per Apple's @c MTLSharedEvent contract.
		 */
		class MetalGpuTimelineFence final : public GpuTimelineFence
		{
		public:
			explicit MetalGpuTimelineFence(MetalGpuDevice& device);
			~MetalGpuTimelineFence() override;

			MetalGpuTimelineFence(const MetalGpuTimelineFence&) = delete;
			MetalGpuTimelineFence& operator=(const MetalGpuTimelineFence&) = delete;

			u64 GetCompletedValue() const final;

#ifdef __OBJC__
			/** Returns the underlying shared event used to signal completion, or @c nil if construction failed. */
			id<MTLSharedEvent> GetSharedEvent() const;
#endif

		protected:
			/**
			 * Native blocking wait on the fence's MTLSharedEvent via an MTLSharedEventListener +
			 * dispatch semaphore. Invoked by Wait(). Replaces the base class' poll-with-sleep fallback
			 * (mirrors VulkanGpuTimelineFence::WaitInternal, which uses vkWaitSemaphores).
			 */
			void WaitInternal(u64 value) final;

		private:
			struct Impl;
			TUnique<Impl> mImpl;
		};

		/** @} */
	} // namespace render
} // namespace b3d
