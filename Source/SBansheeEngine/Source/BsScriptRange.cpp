//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptRange.h"

namespace BansheeEngine
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