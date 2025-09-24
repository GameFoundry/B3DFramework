//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** @copydoc OcclusionQuery */
		class VulkanOcclusionQuery : public OcclusionQuery
		{
		public:
			VulkanOcclusionQuery(VulkanGpuDevice& device, bool binary);
			~VulkanOcclusionQuery() override;

			void Begin(GpuCommandBuffer& commandBuffer) override;
			void End(GpuCommandBuffer& commandBuffer) override;
			bool IsReady() const override;
			u32 GetSampleCount() override;

			/** Returns true if the query begin() was called, but not end(). */
			bool IsInProgress() const;

			/**
			 * Interrupts an in-progress query allowing the command buffer to submitted. Gets called on queries that are still
			 * open during command buffer submission.
			 */
			void Interrupt(VulkanGpuCommandBuffer& commandBuffer);

		private:
			VulkanGpuDevice& mDevice;
			Vector<VulkanQuery*> mQueries;

			u64 mNumSamples = 0;
			bool mQueryEndCalled : 1;
			bool mQueryFinalized : 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
