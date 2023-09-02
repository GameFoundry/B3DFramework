//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptTime.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Utility/BsTime.h"
#include "BsPlayInEditor.h"

using namespace bs;
ScriptTime::ScriptTime(MonoObject* instance)
	: ScriptObject(instance)
{}

void ScriptTime::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_GetRealElapsed", (void*)&ScriptTime::InternalGetRealElapsed);
	metaData.ScriptClass->AddInternalCall("Internal_GetElapsed", (void*)&ScriptTime::InternalGetElapsed);
	metaData.ScriptClass->AddInternalCall("Internal_GetFrameDelta", (void*)&ScriptTime::InternalGetFrameDelta);
	metaData.ScriptClass->AddInternalCall("Internal_GetFrameNumber", (void*)&ScriptTime::InternalGetFrameNumber);
	metaData.ScriptClass->AddInternalCall("Internal_GetPrecise", (void*)&ScriptTime::InternalGetPrecise);
}

float ScriptTime::InternalGetRealElapsed()
{
	return GetTime().GetTime();
}

float ScriptTime::InternalGetElapsed()
{
	return PlayInEditor::Instance().GetPausableTime();
}

float ScriptTime::InternalGetFrameDelta()
{
	return GetTime().GetFrameDelta();
}

u64 ScriptTime::InternalGetFrameNumber()
{
	return GetTime().GetCurrentFrameIndex();
}

u64 ScriptTime::InternalGetPrecise()
{
	return GetTime().GetTimePrecise();
}
