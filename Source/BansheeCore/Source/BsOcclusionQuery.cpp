//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsOcclusionQuery.h"
#include "BsQueryManager.h"

namespace BansheeEngine
{
	OcclusionQuery::OcclusionQuery(bool binary)
		:mActive(false), mBinary(binary)
	{ }

	SPtr<OcclusionQuery> OcclusionQuery::create(bool binary, UINT32 deviceIdx)
	{
		return QueryManager::instance().createOcclusionQuery(binary, deviceIdx);
	}
}