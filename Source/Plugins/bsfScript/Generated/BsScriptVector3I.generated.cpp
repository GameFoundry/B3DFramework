//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVector3I.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector3I::ScriptVector3I(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptVector3I::initRuntimeData()
	{ }

	MonoObject*ScriptVector3I::box(const Vector3I& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Vector3I ScriptVector3I::unbox(MonoObject* value)
	{
		return *(Vector3I*)MonoUtil::unbox(value);
	}

}
