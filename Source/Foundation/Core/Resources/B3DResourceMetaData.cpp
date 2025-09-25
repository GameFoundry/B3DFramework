//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/B3DResourceMetaData.h"
#include "RTTI/B3DResourceMetaDataRTTI.h"

using namespace b3d;

RTTIType* ResourceMetaData::GetRttiStatic()
{
	return ResourceMetaDataRTTI::Instance();
}

RTTIType* ResourceMetaData::GetRtti() const
{
	return ResourceMetaData::GetRttiStatic();
}
