//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGLPrerequisites.h"
#include "Managers/BsQueryManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup GL
		 *  @{
		 */

		/**	Handles creation and life of OpenGL queries. */
		class GLQueryManager : public QueryManager
		{
		public:
			/** @copydoc QueryManager::createEventQuery */
			SPtr<EventQuery> CreateEventQuery(u32 deviceIdx = 0) const;

			/** @copydoc QueryManager::createTimerQuery */
			SPtr<TimerQuery> CreateTimerQuery(u32 deviceIdx = 0) const;

			/** @copydoc QueryManager::createOcclusionQuery */
			SPtr<OcclusionQuery> CreateOcclusionQuery(bool binary, u32 deviceIdx = 0) const;
		};

		/** @} */
	} // namespace ct
} // namespace bs
