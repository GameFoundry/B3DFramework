//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMeta.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"

using namespace bs;
ScriptMeta::ScriptMeta()
{
}

ScriptMeta::ScriptMeta(const String& assembly, const String& ns, const String& name, std::function<void()> initializeBindingsCallback)
	: Ns(ns), Name(name), Assembly(assembly), InitializeBindingsCallback(std::move(initializeBindingsCallback))
{
}
