//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DNullPrerequisites.h"
#include "RenderAPI/B3DGpuCommandBuffer.h"

namespace b3d
{
	namespace render
	{
		class NullGpuDevice;

		/** @addtogroup NullGpuBackend 
		 *  @{
		 */

		/** Null implementation of GpuCommandBufferPool. */
		class NullGpuCommandBufferPool : public GpuCommandBufferPool
		{
			using Base = GpuCommandBufferPool;
		public:
			NullGpuCommandBufferPool(NullGpuDevice& device, const GpuCommandBufferPoolCreateInformation& createInformation);
			~NullGpuCommandBufferPool() override;

			SPtr<GpuCommandBuffer> Create(const GpuCommandBufferCreateInformation& createInformation) override;
			SPtr<GpuCommandBuffer> FindOrCreate(const GpuCommandBufferCreateInformation& createInformation) override;
			void Reset() override {}
			void Destroy() override;

		private:
			u32 mNextCommandBufferId = 1;
			UnorderedMap<u32, SPtr<GpuCommandBuffer>> mCommandBuffers;
		};

		/** @} */
	} // namespace render
} // namespace b3d
