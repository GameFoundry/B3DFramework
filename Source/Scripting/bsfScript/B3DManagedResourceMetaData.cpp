//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsManagedResourceMetaData.h"
#include "RTTI/BsManagedResourceMetaDataRTTI.h"

using namespace b3d;
RTTIType* ManagedResourceMetaData::GetRttiStatic()
{
	return ManagedResourceMetaDataRTTI::Instance();
}

RTTIType* ManagedResourceMetaData::GetRtti() const
{
	return ManagedResourceMetaData::GetRttiStatic();
}
