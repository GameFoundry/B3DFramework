//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsImportOptions.h"
#include "Private/RTTI/BsImportOptionsRTTI.h"

using namespace b3d;

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* ImportOptions::GetRttiStatic()
{
	return ImportOptionsRTTI::Instance();
}

RTTIType* ImportOptions::GetRtti() const
{
	return ImportOptions::GetRttiStatic();
}
