//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/B3DShaderImportOptions.h"
#include "RTTI/B3DShaderImportOptionsRTTI.h"

using namespace b3d;

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* ShaderImportOptions::GetRttiStatic()
{
	return ShaderImportOptionsRTTI::Instance();
}

RTTIType* ShaderImportOptions::GetRtti() const
{
	return ShaderImportOptions::GetRttiStatic();
}
