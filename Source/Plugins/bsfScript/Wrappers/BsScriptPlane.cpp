//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptPlane.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptPlane::ScriptPlane(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPlane::initRuntimeData()
	{ }

	MonoObject* ScriptPlane::box(const Plane& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Plane ScriptPlane::unbox(MonoObject* obj)
	{
		return *(Plane*)MonoUtil::unbox(obj);
	}
}