//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsShaderImportOptions.h"
#include "Private/RTTI/BsShaderImportOptionsRTTI.h"

using namespace bs;

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* ShaderImportOptions::GetRttiStatic()
{
	return ShaderImportOptionsRTTI::Instance();
}

RTTITypeBase* ShaderImportOptions::GetRtti() const
{
	return ShaderImportOptions::GetRttiStatic();
}
