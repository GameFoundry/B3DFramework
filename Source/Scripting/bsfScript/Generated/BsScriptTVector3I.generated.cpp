//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTVector3I.generated.h"
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

	MonoObject*ScriptVector3I::Box(const TVector3I<int32_t>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TVector3I<int32_t> ScriptVector3I::Unbox(MonoObject* value)
	{
		return *(TVector3I<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptVector3UI::ScriptVector3UI(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptVector3UI::InitRuntimeData()
	{ }

	MonoObject*ScriptVector3UI::Box(const TVector3I<uint32_t>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TVector3I<uint32_t> ScriptVector3UI::Unbox(MonoObject* value)
	{
		return *(TVector3I<uint32_t>*)MonoUtil::Unbox(value);
	}

}
