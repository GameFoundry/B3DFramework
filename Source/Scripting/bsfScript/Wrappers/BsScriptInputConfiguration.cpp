//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptInputConfiguration.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Input/BsVirtualInput.h"

using namespace bs;

ScriptVirtualAxis::ScriptVirtualAxis(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptVirtualAxis::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_InitVirtualAxis", (void*)&ScriptVirtualAxis::InternalInitVirtualAxis);
}

u32 ScriptVirtualAxis::InternalInitVirtualAxis(MonoString* name)
{
	String nameStr = MonoUtil::MonoToString(name);

	VirtualAxis vb(nameStr);
	return vb.AxisIdentifier;
}
