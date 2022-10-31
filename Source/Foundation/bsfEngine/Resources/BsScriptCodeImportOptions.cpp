//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Resources/BsScriptCodeImportOptions.h"
#include "Private/RTTI/BsScriptCodeImportOptionsRTTI.h"

using namespace bs;

SPtr<ScriptCodeImportOptions> ScriptCodeImportOptions::Create()
{
	return B3DMakeShared<ScriptCodeImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* ScriptCodeImportOptions::GetRttiStatic()
{
	return ScriptCodeImportOptionsRTTI::Instance();
}

RTTITypeBase* ScriptCodeImportOptions::GetRtti() const
{
	return ScriptCodeImportOptions::GetRttiStatic();
}
