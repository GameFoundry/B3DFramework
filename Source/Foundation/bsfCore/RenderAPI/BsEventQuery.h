//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup RenderAPI-Internal
		 *  @{
		 */

		/** Represents a GPU query that gets signaled when GPU starts processing the query. */
		class B3D_CORE_EXPORT EventQuery
		{
		public:
			EventQuery() = default;
			virtual ~EventQuery() = default;

			/** Schedules the query for execution on the command buffer. Once the GPU reaches this point the query will be set in the signaled state. */
			virtual void Begin(const SPtr<CommandBuffer>& cb = nullptr) = 0;

			/**	Checks if query results are ready. */
			virtual bool IsReady() const = 0;
		};

		/** @} */
	} // namespace ct
} // namespace bs
