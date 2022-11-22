//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsTimerQuery.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** @copydoc TimerQuery */
		class VulkanTimerQuery : public TimerQuery
		{
		public:
			VulkanTimerQuery(VulkanDevice& device);
			~VulkanTimerQuery() override;

			void Begin(const SPtr<CommandBuffer>& cb) override;
			void End(const SPtr<CommandBuffer>& cb) override;
			bool IsReady() const override;
			float GetTimeMs() override;

			/** Returns true if the query begin() was called, but not end(). */
			bool IsInProgress() const;

			/**
			 * Interrupts an in-progress query allowing the command buffer to submitted. Gets called on queries that are still
			 * open during command buffer submission.
			 */
			void Interrupt(VulkanCmdBuffer& cb);

		private:
			VulkanDevice& mDevice;
			Vector<std::pair<VulkanQuery*, VulkanQuery*>> mQueries;

			float mTimeDelta = 0.0f;
			bool mQueryEndCalled : 1;
			bool mQueryFinalized : 1;
		};

		/** @} */
	} // namespace ct
} // namespace bs
