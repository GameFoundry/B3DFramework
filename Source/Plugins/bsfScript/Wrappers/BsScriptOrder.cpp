//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptOrder.h"

namespace bs
{
	MonoField* ScriptOrder::indexField = nullptr;

	ScriptOrder::ScriptOrder(MonoObject* instance) : ScriptObject(instance)
	{ }

	void ScriptOrder::initRuntimeData()
	{
		indexField = metaData.scriptClass->getField("index");
	}
}