//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsOcclusionQuery.h"

namespace bs
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
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc OcclusionQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override;

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override;

		/** @copydoc OcclusionQuery::getNumSamples */
		UINT32 getNumSamples() override;

	private:
		friend class QueryManager;

		VulkanDevice& mDevice;
		VulkanQuery* mQuery;

		UINT64 mNumSamples;
		bool mQueryEndCalled : 1;
		bool mQueryFinalized : 1;
	};

	/** @} */
}