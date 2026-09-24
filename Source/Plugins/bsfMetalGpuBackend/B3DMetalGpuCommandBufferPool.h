//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DMetalPrerequisites.h"
#include "GpuBackend/B3DGpuCommandBuffer.h"

namespace b3d
{
	namespace render
	{
		class MetalGpuDevice;

		/** @addtogroup MetalGpuBackend
		 *  @{
		 */

		/** Metal implementation of GpuCommandBufferPool. */
		class MetalGpuCommandBufferPool : public GpuCommandBufferPool
		{
			using Base = GpuCommandBufferPool;
		public:
			MetalGpuCommandBufferPool(MetalGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation);
			~MetalGpuCommandBufferPool() override;

			TShared<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) override;
			TShared<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) override;

			void Reset() override;
			void Destroy() override;

			/**
			 * Notifies the pool that a command buffer has completed on the GPU and is safe to recycle.
			 *
			 * Must be called on the pool's owner thread.
			 */
			void NotifyCommandBufferReady(u32 id);

		private:
			u32 mNextCommandBufferId = 1;
			UnorderedMap<u32, TShared<GpuCommandBuffer>> mCommandBuffers;

			// Recycle free-list. Populated by NotifyCommandBufferReady from the completion handler
			// (and rebuilt by Reset), drained by FindOrCreate.
			Vector<u32> mReadyIds;
		};

		/** @} */
	} // namespace render
} // namespace b3d
