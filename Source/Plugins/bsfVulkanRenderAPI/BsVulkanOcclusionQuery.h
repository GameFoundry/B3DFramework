//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs { namespace ct
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
		void begin(const SPtr<CommandBuffer>& cb) override;

		/** @copydoc OcclusionQuery::end */
		void end(const SPtr<CommandBuffer>& cb) override;

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override;

		/** @copydoc OcclusionQuery::getNumSamples */
		UINT32 getNumSamples() override;

		/** Returns true if the query begin() was called, but not end(). */
		bool _isInProgress() const;

		/**
		 * Interrupts an in-progress query, allowing the command buffer to submitted. Interrupted queries must be resumed
		 * using _resume().
		 */
		void _interrupt(VulkanCmdBuffer& cb);

		/** Resumes an interrupted query, restoring it back to its original in-progress state. */
		void _resume(VulkanCmdBuffer& cb);

	private:
		friend class QueryManager;

		VulkanDevice& mDevice;
		Vector<VulkanQuery*> mQueries;

		UINT64 mNumSamples = 0;
		bool mQueryEndCalled : 1;
		bool mQueryFinalized : 1;
		bool mQueryInterrupted : 1;
	};

	/** @} */
}}