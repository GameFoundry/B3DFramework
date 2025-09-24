//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DVulkanPrerequisites.h"
#include "RenderAPI/B3DTimerQuery.h"

namespace b3d
{
	namespace render
	{
		/** @addtogroup Vulkan
		 *  @{
		 */

		/** @copydoc TimerQuery */
		class VulkanTimerQuery : public TimerQuery
		{
		public:
			VulkanTimerQuery(VulkanGpuDevice& device);
			~VulkanTimerQuery() override;

			void Begin(GpuCommandBuffer& commandBuffer) override;
			void End(GpuCommandBuffer& commandBuffer) override;
			bool IsReady() const override;
			float GetTimeMs() override;

			/** Returns true if the query begin() was called, but not end(). */
			bool IsInProgress() const;

			/**
			 * Interrupts an in-progress query allowing the command buffer to submitted. Gets called on queries that are still
			 * open during command buffer submission.
			 */
			void Interrupt(VulkanGpuCommandBuffer& commandBuffer);

		private:
			VulkanGpuDevice& mDevice;
			Vector<std::pair<VulkanQuery*, VulkanQuery*>> mQueries;

			float mTimeDelta = 0.0f;
			bool mQueryEndCalled : 1;
			bool mQueryFinalized : 1;
		};

		/** @} */
	} // namespace render
} // namespace b3d
