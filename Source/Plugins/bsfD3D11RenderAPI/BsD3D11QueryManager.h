//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsD3D11Prerequisites.h"
#include "Managers/BsQueryManager.h"

namespace bs
{
	namespace ct
	{
		/** @addtogroup D3D11
		 *  @{
		 */

		/**	Handles creation of DirectX 11 queries. */
		class D3D11QueryManager : public QueryManager
		{
		public:
			SPtr<EventQuery> CreateEventQuery(u32 deviceIdx = 0) const override;
			SPtr<TimerQuery> CreateTimerQuery(u32 deviceIdx = 0) const override;
			SPtr<OcclusionQuery> CreateOcclusionQuery(bool binary, u32 deviceIdx = 0) const override;
		};

		/** @} */
	} // namespace ct
} // namespace bs
