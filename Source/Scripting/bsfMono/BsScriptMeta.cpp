//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMeta.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"

namespace bs
{
	ScriptMeta::ScriptMeta()
		: ScriptClass(nullptr), ThisPtrField(nullptr)
	{
	}

	ScriptMeta::ScriptMeta(const String& assembly, const String& ns, const String& name, std::function<void()> initCallback)
		: Ns(ns), Name(name), Assembly(assembly), InitCallback(initCallback), ScriptClass(nullptr), ThisPtrField(nullptr)
	{
	}
} // namespace bs
