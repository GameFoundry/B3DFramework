//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsManagedResourceMetaData.h"
#include "BsManagedResourceMetaDataRTTI.h"

namespace bs
{
	RTTITypeBase* ManagedResourceMetaData::getRTTIStatic()
	{
		return ManagedResourceMetaDataRTTI::instance();
	}

	RTTITypeBase* ManagedResourceMetaData::getRTTI() const
	{
		return ManagedResourceMetaData::getRTTIStatic();
	}
}