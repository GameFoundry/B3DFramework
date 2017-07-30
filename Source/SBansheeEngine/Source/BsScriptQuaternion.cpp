//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptQuaternion.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptQuaternion::ScriptQuaternion(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptQuaternion::initRuntimeData()
	{ }

	MonoObject* ScriptQuaternion::box(const Quaternion& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Quaternion ScriptQuaternion::unbox(MonoObject* obj)
	{
		return *(Quaternion*)MonoUtil::unbox(obj);
	}
}