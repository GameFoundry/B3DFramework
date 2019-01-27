//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptLogEntry.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptLogEntry::ScriptLogEntry(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptLogEntry::initRuntimeData()
	{ }
}