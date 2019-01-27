//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpring.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSpring::ScriptSpring(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSpring::initRuntimeData()
	{ }

	MonoObject*ScriptSpring::box(const Spring& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	Spring ScriptSpring::unbox(MonoObject* value)
	{
		return *(Spring*)MonoUtil::unbox(value);
	}

}
