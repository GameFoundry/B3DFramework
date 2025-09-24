//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsScriptCodeImportOptions.h"
#include "Private/RTTI/BsScriptCodeImportOptionsRTTI.h"

using namespace b3d;

SPtr<ScriptCodeImportOptions> ScriptCodeImportOptions::Create()
{
	return B3DMakeShared<ScriptCodeImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* ScriptCodeImportOptions::GetRttiStatic()
{
	return ScriptCodeImportOptionsRTTI::Instance();
}

RTTIType* ScriptCodeImportOptions::GetRtti() const
{
	return ScriptCodeImportOptions::GetRttiStatic();
}
