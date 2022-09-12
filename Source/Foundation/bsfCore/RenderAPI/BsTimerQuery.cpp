//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsTimerQuery.h"
#include "Managers/BsQueryManager.h"

namespace bs { namespace ct
{
	SPtr<TimerQuery> TimerQuery::Create(UINT32 deviceIdx)
	{
		return QueryManager::Instance().CreateTimerQuery(deviceIdx);
	}
}}
