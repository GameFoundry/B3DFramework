//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsEventQuery.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc EventQuery */
	class VulkanEventQuery : public EventQuery
	{
	public:
		VulkanEventQuery(UINT32 deviceIdx);
		~VulkanEventQuery();

		/** @copydoc EventQuery::begin */
		void begin() override;

		/** @copydoc EventQuery::isReady */
		bool isReady() const override;

	private:
	};

	/** @} */
}