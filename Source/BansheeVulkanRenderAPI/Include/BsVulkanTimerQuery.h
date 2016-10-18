//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsTimerQuery.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc TimerQuery */
	class VulkanTimerQuery : public TimerQuery
	{
	public:
		VulkanTimerQuery(UINT32 deviceIdx);
		~VulkanTimerQuery();

		/** @copydoc TimerQuery::begin */
		void begin() override;

		/** @copydoc TimerQuery::end */
		void end() override;

		/** @copydoc TimerQuery::isReady */
		bool isReady() const override;

		/** @copydoc TimerQuery::getTimeMs */
		float getTimeMs() override;

	private:
	};

	/** @} */
}