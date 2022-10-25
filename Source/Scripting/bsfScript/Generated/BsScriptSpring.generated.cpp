//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSpring.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSpring::ScriptSpring(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptSpring::InitRuntimeData()
	{}

	MonoObject* ScriptSpring::Box(const Spring& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	Spring ScriptSpring::Unbox(MonoObject* value)
	{
		return *(Spring*)MonoUtil::Unbox(value);
	}

} // namespace bs
