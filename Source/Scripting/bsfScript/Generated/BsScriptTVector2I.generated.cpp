//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTVector2I.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptVector2I::ScriptVector2I(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptVector2I::InitRuntimeData()
	{ }

	MonoObject*ScriptVector2I::Box(const TVector2I<int32_t>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TVector2I<int32_t> ScriptVector2I::Unbox(MonoObject* value)
	{
		return *(TVector2I<int32_t>*)MonoUtil::Unbox(value);
	}


	ScriptVector2UI::ScriptVector2UI(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptVector2UI::InitRuntimeData()
	{ }

	MonoObject*ScriptVector2UI::Box(const TVector2I<uint32_t>& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	TVector2I<uint32_t> ScriptVector2UI::Unbox(MonoObject* value)
	{
		return *(TVector2I<uint32_t>*)MonoUtil::Unbox(value);
	}

}
