//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsEventQuery.h"
#include "BsQueryManager.h"

namespace bs
{
	SPtr<EventQuery> EventQuery::create(UINT32 deviceIdx)
	{
		return QueryManager::instance().createEventQuery(deviceIdx);
	}
}