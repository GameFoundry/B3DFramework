//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsOcclusionQuery.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/** @copydoc OcclusionQuery */
	class VulkanOcclusionQuery : public OcclusionQuery
	{
	public:
		VulkanOcclusionQuery(bool binary, UINT32 deviceIdx);
		~VulkanOcclusionQuery();

		/** @copydoc OcclusionQuery::begin */
		void begin() override;

		/** @copydoc OcclusionQuery::end */
		void end() override;

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override;

		/** @copydoc OcclusionQuery::getNumSamples */
		UINT32 getNumSamples() override;

	private:
		friend class QueryManager;
	};

	/** @} */
}