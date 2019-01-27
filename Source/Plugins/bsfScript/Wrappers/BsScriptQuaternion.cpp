//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptQuaternion.h"
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