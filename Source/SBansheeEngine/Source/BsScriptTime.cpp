//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptTime.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "BsTime.h"
#include "BsPlayInEditorManager.h"

namespace bs
{
	ScriptTime::ScriptTime(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptTime::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRealElapsed", &ScriptTime::internal_getRealElapsed);
		metaData.scriptClass->addInternalCall("Internal_GetElapsed", &ScriptTime::internal_getElapsed);
		metaData.scriptClass->addInternalCall("Internal_GetFrameDelta", &ScriptTime::internal_getFrameDelta);
		metaData.scriptClass->addInternalCall("Internal_GetFrameNumber", &ScriptTime::internal_getFrameNumber);
		metaData.scriptClass->addInternalCall("Internal_GetPrecise", &ScriptTime::internal_getPrecise);
	}

	float ScriptTime::internal_getRealElapsed()
	{
		return gTime().getTime();
	}

	float ScriptTime::internal_getElapsed()
	{
		return PlayInEditorManager::instance().getPausableTime();
	}

	float ScriptTime::internal_getFrameDelta()
	{
		return gTime().getFrameDelta();
	}

	UINT64 ScriptTime::internal_getFrameNumber()
	{
		return gTime().getFrameIdx();
	}

	UINT64 ScriptTime::internal_getPrecise()
	{
		return gTime().getTimePrecise();
	}
}