//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptKerningPair.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptKerningPair::ScriptKerningPair(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptKerningPair::InitRuntimeData()
	{ }

	MonoObject*ScriptKerningPair::Box(const KerningPair& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	KerningPair ScriptKerningPair::Unbox(MonoObject* value)
	{
		return *(KerningPair*)MonoUtil::Unbox(value);
	}

}
