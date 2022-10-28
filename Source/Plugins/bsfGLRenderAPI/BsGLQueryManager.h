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
			SPtr<EventQuery> CreateEventQuery(u32 deviceIdx = 0) const override;
			SPtr<TimerQuery> CreateTimerQuery(u32 deviceIdx = 0) const override;
			SPtr<OcclusionQuery> CreateOcclusionQuery(bool binary, u32 deviceIdx = 0) const override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
