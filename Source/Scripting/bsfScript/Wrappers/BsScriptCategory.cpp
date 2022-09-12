//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptCategory.h"

namespace bs
{
	MonoField* ScriptCategory::nameField = nullptr;

	ScriptCategory::ScriptCategory(MonoObject* instance) : ScriptObject(instance)
	{ }

	void ScriptCategory::InitRuntimeData()
	{
		nameField = metaData.scriptClass->GetField("name");
	}
}
