//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptStep.h"

namespace bs
{
	MonoField* ScriptStep::stepField = nullptr;

	ScriptStep::ScriptStep(MonoObject* instance) : ScriptObject(instance)
	{ }

	void ScriptStep::initRuntimeData()
	{
		stepField = metaData.scriptClass->getField("step");
	}
}
