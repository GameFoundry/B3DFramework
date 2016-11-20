//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsTimerQuery.h"

namespace bs
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc TimerQuery */
	class VulkanTimerQuery : public TimerQuery
	{
	public:
		VulkanTimerQuery(VulkanDevice& device);
		~VulkanTimerQuery();

		/** @copydoc TimerQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc TimerQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc TimerQuery::isReady */
		bool isReady() const override;

		/** @copydoc TimerQuery::getTimeMs */
		float getTimeMs() override;

	private:
		VulkanDevice& mDevice;
		VulkanQuery* mBeginQuery;
		VulkanQuery* mEndQuery;

		float mTimeDelta;
		bool mQueryEndCalled : 1;
		bool mQueryFinalized : 1;
	};

	/** @} */
}