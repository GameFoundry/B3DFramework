//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIOption.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptGUIOption::ScriptGUIOption(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIOption::InitRuntimeData()
	{ }

	MonoObject*ScriptGUIOption::Box(const GUIOption& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	GUIOption ScriptGUIOption::Unbox(MonoObject* value)
	{
		return *(GUIOption*)MonoUtil::Unbox(value);
	}

}
