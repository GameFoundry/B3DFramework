//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptTime.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoMethod.h"
#include "BsMonoUtil.h"
#include "Utility/BsTime.h"
#include "BsPlayInEditor.h"

namespace bs
{
	ScriptTime::ScriptTime(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptTime::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetRealElapsed", (void*)&ScriptTime::internal_getRealElapsed);
		metaData.scriptClass->addInternalCall("Internal_GetElapsed", (void*)&ScriptTime::internal_getElapsed);
		metaData.scriptClass->addInternalCall("Internal_GetFrameDelta", (void*)&ScriptTime::internal_getFrameDelta);
		metaData.scriptClass->addInternalCall("Internal_GetFrameNumber", (void*)&ScriptTime::internal_getFrameNumber);
		metaData.scriptClass->addInternalCall("Internal_GetPrecise", (void*)&ScriptTime::internal_getPrecise);
	}

	float ScriptTime::internal_getRealElapsed()
	{
		return gTime().getTime();
	}

	float ScriptTime::internal_getElapsed()
	{
		return PlayInEditor::instance().getPausableTime();
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