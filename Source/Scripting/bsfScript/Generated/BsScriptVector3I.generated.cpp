//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	void ScriptVector3I::InitRuntimeData()
	{ }

	MonoObject*ScriptVector3I::Box(const Vector3I& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	Vector3I ScriptVector3I::Unbox(MonoObject* value)
	{
		return *(Vector3I*)MonoUtil::Unbox(value);
	}

}
