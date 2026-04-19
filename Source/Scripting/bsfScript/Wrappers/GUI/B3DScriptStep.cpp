//********************************* B3D Framework - Copyright 2026 Marko Pintera *****************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/B3DScriptStep.h"

#include "B3DMonoClass.h"

using namespace b3d;

MonoField* ScriptStep::sStepField = nullptr;

ScriptStep::ScriptStep()
{
	
}

void ScriptStep::SetupScriptBindings()
{
	sStepField = sInteropMetaData.ScriptClass->GetField("step");
}
