//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsManagedResourceMetaData.h"
#include "RTTI/BsManagedResourceMetaDataRTTI.h"

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