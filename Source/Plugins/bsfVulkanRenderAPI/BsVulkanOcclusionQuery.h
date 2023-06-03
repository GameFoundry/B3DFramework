//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs
{
	namespace ct
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

		private:
			VulkanGpuDevice& mDevice;
			Vector<VulkanQuery*> mQueries;

			u64 mNumSamples = 0;
			bool mQueryEndCalled : 1;
			bool mQueryFinalized : 1;
		};

		/** @} */
	} // namespace ct
} // namespace bs
