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
			VulkanOcclusionQuery(VulkanDevice& device, bool binary);
			~VulkanOcclusionQuery();

			/** @copydoc OcclusionQuery::begin */
			void Begin(const SPtr<CommandBuffer>& cb);

			/** @copydoc OcclusionQuery::end */
			void End(const SPtr<CommandBuffer>& cb);

			/** @copydoc OcclusionQuery::isReady */
			bool IsReady() const override;

			/** @copydoc OcclusionQuery::getNumSamples */
			u32 GetNumSamples() override;

			/** Returns true if the query begin() was called, but not end(). */
			bool IsInProgressInternal() const;

			/**
			 * Interrupts an in-progress query allowing the command buffer to submitted. Gets called on queries that are still
			 * open during command buffer submission.
			 */
			void InterruptInternal(VulkanCmdBuffer& cb);

		private:
			friend class QueryManager;

			VulkanDevice& mDevice;
			Vector<VulkanQuery*> mQueries;

			u64 mNumSamples = 0;
			bool mQueryEndCalled : 1;
			bool mQueryFinalized : 1;
		};

		/** @} */
	} // namespace ct
} // namespace bs
