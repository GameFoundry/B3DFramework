//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "RenderAPI/BsOcclusionQuery.h"
#include "Managers/BsQueryManager.h"

namespace bs { namespace ct
{
	OcclusionQuery::OcclusionQuery(bool binary)
		:mActive(false), mBinary(binary)
	{ }

	SPtr<OcclusionQuery> OcclusionQuery::Create(bool binary, UINT32 deviceIdx)
	{
		return QueryManager::Instance().CreateOcclusionQuery(binary, deviceIdx);
	}
}}
