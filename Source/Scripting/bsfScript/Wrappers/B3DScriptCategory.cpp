//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptCategory.h"
#include "BsMonoClass.h"

using namespace b3d;

MonoField* ScriptCategory::sNameField = nullptr;

ScriptCategory::ScriptCategory()
{
	
}

void ScriptCategory::SetupScriptBindings()
{
	sNameField = sInteropMetaData.ScriptClass->GetField("name");
}
