//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptRange.h"

namespace bs
{
	MonoField* ScriptRange::sliderField = nullptr;
	MonoField* ScriptRange::minRangeField = nullptr;
	MonoField* ScriptRange::maxRangeField = nullptr;

	ScriptRange::ScriptRange(MonoObject* instance) : ScriptObject(instance)
	{ }
	void ScriptRange::initRuntimeData()
	{
		minRangeField = metaData.scriptClass->getField("min");
		maxRangeField = metaData.scriptClass->getField("max");
		sliderField = metaData.scriptClass->getField("slider");
	}
}