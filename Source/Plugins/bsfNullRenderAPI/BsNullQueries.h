//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsQueryManager.h"
#include "RenderAPI/BsEventQuery.h"
#include "RenderAPI/BsTimerQuery.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs { namespace ct
{
	/** @addtogroup NullRenderAPI
	 *  @{
	 */

	/**	Handles creation of null queries. */
	class NullQueryManager final : public QueryManager
	{
	public:
		/** @copydoc QueryManager::createEventQuery */
		SPtr<EventQuery> createEventQuery(UINT32 deviceIdx = 0) const override;

		/** @copydoc QueryManager::createTimerQuery */
		SPtr<TimerQuery> createTimerQuery(UINT32 deviceIdx = 0) const override;

		/** @copydoc QueryManager::createOcclusionQuery */
		SPtr<OcclusionQuery> createOcclusionQuery(bool binary, UINT32 deviceIdx = 0) const override;
	};

	/** @copydoc EventQuery */
	class NullEventQuery final : public EventQuery
	{
	public:
		NullEventQuery() = default;

		/** @copydoc EventQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override { }

		/** @copydoc EventQuery::isReady */
		bool isReady() const override { return true; }
	};

	/** @copydoc TimerQuery */
	class NullTimerQuery final : public TimerQuery
	{
	public:
		NullTimerQuery() = default;

		/** @copydoc TimerQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override { }

		/** @copydoc TimerQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override { }

		/** @copydoc TimerQuery::isReady */
		bool isReady() const override { return true; }

		/** @copydoc TimerQuery::getTimeMs */
		float getTimeMs() override { return 0.0f; }
	};

	/** @copydoc OcclusionQuery */
	class NullOcclusionQuery final : public OcclusionQuery
	{
	public:
		NullOcclusionQuery(bool binary)
			:OcclusionQuery(binary)
		{ }

		/** @copydoc OcclusionQuery::begin */
		void begin(const SPtr<CommandBuffer>& cb = nullptr) override { }

		/** @copydoc OcclusionQuery::end */
		void end(const SPtr<CommandBuffer>& cb = nullptr) override { }

		/** @copydoc OcclusionQuery::isReady */
		bool isReady() const override { return true; }

		/** @copydoc OcclusionQuery::getNumSamples */
		UINT32 getNumSamples() override { return 0; }

	private:
		friend class QueryManager;
	};

	/** @} */
}}
