//********************************* B3D Framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/B3DScriptRange.h"
#include "B3DMonoClass.h"

using namespace b3d;

MonoField* ScriptRange::sSliderField = nullptr;
MonoField* ScriptRange::sMinRangeField = nullptr;
MonoField* ScriptRange::sMaxRangeField = nullptr;

ScriptRange::ScriptRange()
{
	
} 

void ScriptRange::SetupScriptBindings()
{
	sMinRangeField = sInteropMetaData.ScriptClass->GetField("min");
	sMaxRangeField = sInteropMetaData.ScriptClass->GetField("max");
	sSliderField = sInteropMetaData.ScriptClass->GetField("slider");
}
