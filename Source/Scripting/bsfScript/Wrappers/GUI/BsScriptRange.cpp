//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptRange.h"
#include "BsMonoClass.h"

using namespace bs;

MonoField* ScriptRange::sSliderField = nullptr;
MonoField* ScriptRange::sMinRangeField = nullptr;
MonoField* ScriptRange::sMaxRangeField = nullptr;

void ScriptRange::SetupScriptBindings()
{
	sMinRangeField = sInteropMetaData.ScriptClass->GetField("min");
	sMaxRangeField = sInteropMetaData.ScriptClass->GetField("max");
	sSliderField = sInteropMetaData.ScriptClass->GetField("slider");
}
