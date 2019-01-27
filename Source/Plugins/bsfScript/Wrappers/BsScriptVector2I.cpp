//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/BsScriptVector2I.h"
#include "BsMonoManager.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector2I::ScriptVector2I(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptVector2I::initRuntimeData()
	{ }

	MonoObject* ScriptVector2I::box(const Vector2I& value)
	{
		// We're casting away const but it's fine since structs are passed by value anyway
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector2I ScriptVector2I::unbox(MonoObject* obj)
	{
		return *(Vector2I*)MonoUtil::unbox(obj);
	}
}