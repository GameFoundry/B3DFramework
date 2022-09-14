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

	void ScriptRectOffset::initRuntimeData()
	{ }

	MonoObject*ScriptRectOffset::box(const RectOffset& value)
	{
		return MonoUtil::box(metaData.scriptClass->GetInternalClassInternal(), (void*)&value);
	}

	RectOffset ScriptRectOffset::unbox(MonoObject* value)
	{
		return *(RectOffset*)MonoUtil::unbox(value);
	}

}
