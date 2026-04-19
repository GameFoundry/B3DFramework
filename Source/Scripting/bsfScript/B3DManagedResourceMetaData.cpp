//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DManagedResourceMetaData.h"
#include "RTTI/B3DManagedResourceMetaDataRTTI.h"

using namespace b3d;
RTTIType* ManagedResourceMetaData::GetRttiStatic()
{
	return ManagedResourceMetaDataRTTI::Instance();
}

RTTIType* ManagedResourceMetaData::GetRtti() const
{
	return ManagedResourceMetaData::GetRttiStatic();
}
