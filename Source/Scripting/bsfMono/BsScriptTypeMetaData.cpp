//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTypeMetaData.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoField.h"

using namespace bs;

ScriptTypeMetaData::ScriptTypeMetaData(const String& assembly, const String& nameSpace, const String& name, std::function<void()> setupScriptBindingsCallback)
	: Namespace(nameSpace), Name(name), Assembly(assembly), SetupScriptBindingsCallback(std::move(setupScriptBindingsCallback))
{
}
