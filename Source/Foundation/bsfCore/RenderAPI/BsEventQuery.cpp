//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsEventQuery.h"
#include "Managers/BsQueryManager.h"

using namespace bs;

namespace bs { namespace ct
{
SPtr<EventQuery> EventQuery::Create(u32 deviceIdx)
{
	return QueryManager::Instance().CreateEventQuery(deviceIdx);
}
}}
