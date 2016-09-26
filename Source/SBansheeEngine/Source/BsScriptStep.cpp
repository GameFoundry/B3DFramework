//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//

#include "BsScriptEnginePrerequisites.h"
#include <BsScriptStep.h>

namespace BansheeEngine
{
	MonoField* ScriptStep::stepField = nullptr;

	ScriptStep::ScriptStep(MonoObject* instance) : ScriptObject(instance)
	{ }
	void ScriptStep::initRuntimeData()
	{
		stepField = metaData.scriptClass->getField("step");
	}
}
