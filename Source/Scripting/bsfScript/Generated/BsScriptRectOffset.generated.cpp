//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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

	MonoObject*ScriptRectOffset::Box(const RectOffset& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	RectOffset ScriptRectOffset::Unbox(MonoObject* value)
	{
		return *(RectOffset*)MonoUtil::Unbox(value);
	}

}
