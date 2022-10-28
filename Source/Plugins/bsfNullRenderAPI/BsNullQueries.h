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
			SPtr<EventQuery> CreateEventQuery(u32 deviceIdx = 0) const override;
			SPtr<TimerQuery> CreateTimerQuery(u32 deviceIdx = 0) const override;
			SPtr<OcclusionQuery> CreateOcclusionQuery(bool binary, u32 deviceIdx = 0) const override;
		};

		/** @copydoc EventQuery */
		class NullEventQuery final : public EventQuery
		{
		public:
			NullEventQuery() = default;

			void Begin(const SPtr<CommandBuffer>& cb = nullptr) override {}
			bool IsReady() const override { return true; }
		};

		/** @copydoc TimerQuery */
		class NullTimerQuery final : public TimerQuery
		{
		public:
			NullTimerQuery() = default;

			void Begin(const SPtr<CommandBuffer>& cb = nullptr) override {}
			void End(const SPtr<CommandBuffer>& cb = nullptr) override {}
			bool IsReady() const override { return true; }
			float GetTimeMs() override { return 0.0f; }
		};

		/** @copydoc OcclusionQuery */
		class NullOcclusionQuery final : public OcclusionQuery
		{
		public:
			NullOcclusionQuery(bool binary)
				: OcclusionQuery(binary)
			{}

			void Begin(const SPtr<CommandBuffer>& cb = nullptr) override {}
			void End(const SPtr<CommandBuffer>& cb = nullptr) override {}
			bool IsReady() const override { return true; }
			u32 GetNumSamples() override { return 0; }

		private:
			friend class QueryManager;
		};

		/** @} */
	} // namespace ct
} // namespace bs
