//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsResourceMetaData.h"
#include "Private/RTTI/BsResourceMetaDataRTTI.h"

namespace bs
{
	RTTITypeBase* ResourceMetaData::getRTTIStatic()
	{
		return ResourceMetaDataRTTI::Instance();
	}

	RTTITypeBase* ResourceMetaData::getRTTI() const
	{
		return ResourceMetaData::GetRTTIStatic();
	}
}
