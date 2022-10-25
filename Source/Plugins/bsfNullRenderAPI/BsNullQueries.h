//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsNullPrerequisites.h"
#include "Managers/BsQueryManager.h"
#include "RenderAPI/BsEventQuery.h"
#include "RenderAPI/BsTimerQuery.h"
#include "RenderAPI/BsOcclusionQuery.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup NullRenderAPI
		 *  @{
		 */

		/**	Handles creation of null queries. */
		class NullQueryManager final : public QueryManager
		{
		public:
			/** @copydoc QueryManager::createEventQuery */
			SPtr<EventQuery> CreateEventQuery(u32 deviceIdx = 0) const;

			/** @copydoc QueryManager::createTimerQuery */
			SPtr<TimerQuery> CreateTimerQuery(u32 deviceIdx = 0) const;

			/** @copydoc QueryManager::createOcclusionQuery */
			SPtr<OcclusionQuery> CreateOcclusionQuery(bool binary, u32 deviceIdx = 0) const;
		};

		/** @copydoc EventQuery */
		class NullEventQuery final : public EventQuery
		{
		public:
			NullEventQuery() = default;

			/** @copydoc EventQuery::begin */
			void Begin(const SPtr<CommandBuffer>& cb = nullptr) {}

			/** @copydoc EventQuery::isReady */
			bool IsReady() const { return true; }
		};

		/** @copydoc TimerQuery */
		class NullTimerQuery final : public TimerQuery
		{
		public:
			NullTimerQuery() = default;

			/** @copydoc TimerQuery::begin */
			void Begin(const SPtr<CommandBuffer>& cb = nullptr) {}

			/** @copydoc TimerQuery::end */
			void End(const SPtr<CommandBuffer>& cb = nullptr) {}

			/** @copydoc TimerQuery::isReady */
			bool IsReady() const { return true; }

			/** @copydoc TimerQuery::getTimeMs */
			float GetTimeMs() { return 0.0f; }
		};

		/** @copydoc OcclusionQuery */
		class NullOcclusionQuery final : public OcclusionQuery
		{
		public:
			NullOcclusionQuery(bool binary)
				: OcclusionQuery(binary)
			{}

			/** @copydoc OcclusionQuery::begin */
			void Begin(const SPtr<CommandBuffer>& cb = nullptr) {}

			/** @copydoc OcclusionQuery::end */
			void End(const SPtr<CommandBuffer>& cb = nullptr) {}

			/** @copydoc OcclusionQuery::isReady */
			bool IsReady() const { return true; }

			/** @copydoc OcclusionQuery::getNumSamples */
			u32 GetNumSamples() { return 0; }

		private:
			friend class QueryManager;
		};

		/** @} */
	} // namespace ct
} // namespace bs
