//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptRange.h"

namespace bs
{
	MonoField* ScriptRange::sliderField = nullptr;
	MonoField* ScriptRange::minRangeField = nullptr;
	MonoField* ScriptRange::maxRangeField = nullptr;

	ScriptRange::ScriptRange(MonoObject* instance)
		: ScriptObject(instance)
	{}

	void ScriptRange::InitRuntimeData()
	{
		minRangeField = metaData.ScriptClass->GetField("min");
		maxRangeField = metaData.ScriptClass->GetField("max");
		sliderField = metaData.ScriptClass->GetField("slider");
	}
} // namespace bs
