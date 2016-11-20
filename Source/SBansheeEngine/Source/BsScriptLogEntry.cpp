//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptLogEntry.h"
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