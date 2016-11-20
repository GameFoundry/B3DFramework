//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsResourceMetaData.h"
#include "BsResourceMetaDataRTTI.h"

namespace bs
{
	RTTITypeBase* ResourceMetaData::getRTTIStatic()
	{
		return ResourceMetaDataRTTI::instance();
	}

	RTTITypeBase* ResourceMetaData::getRTTI() const
	{
		return ResourceMetaData::getRTTIStatic();
	}
}