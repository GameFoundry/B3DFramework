//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptCategory.h"
#include "BsMonoClass.h"

using namespace bs;

MonoField* ScriptCategory::sNameField = nullptr;

ScriptCategory::ScriptCategory()
{
	
}

void ScriptCategory::SetupScriptBindings()
{
	sNameField = sInteropMetaData.ScriptClass->GetField("name");
}
