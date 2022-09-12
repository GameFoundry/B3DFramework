//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptRectOffset.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptRectOffset::ScriptRectOffset(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptRectOffset::InitRuntimeData()
	{ }

	MonoObject*ScriptRectOffset::box(const RectOffset& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	RectOffset ScriptRectOffset::Unbox(MonoObject* value)
	{
		return *(RectOffset*)MonoUtil::unbox(value);
	}

}
