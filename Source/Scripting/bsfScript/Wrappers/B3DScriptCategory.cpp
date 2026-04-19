//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/B3DScriptCategory.h"
#include "B3DMonoClass.h"

using namespace b3d;

MonoField* ScriptCategory::sNameField = nullptr;

ScriptCategory::ScriptCategory()
{
	
}

void ScriptCategory::SetupScriptBindings()
{
	sNameField = sInteropMetaData.ScriptClass->GetField("name");
}
