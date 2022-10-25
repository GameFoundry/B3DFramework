//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMorphShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptMorphShape::ScriptMorphShape(MonoObject* managedInstance, const SPtr<MorphShape>& value)
		: TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptMorphShape::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptMorphShape::InternalGetName);
		metaData.ScriptClass->AddInternalCall("Internal_GetWeight", (void*)&ScriptMorphShape::InternalGetWeight);
	}

	MonoObject* ScriptMorphShape::Create(const SPtr<MorphShape>& value)
	{
		if(value == nullptr) return nullptr;

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new(bs_alloc<ScriptMorphShape>()) ScriptMorphShape(managedInstance, value);
		return managedInstance;
	}

	MonoString* ScriptMorphShape::InternalGetName(ScriptMorphShape* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->GetInternal()->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	float ScriptMorphShape::InternalGetWeight(ScriptMorphShape* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->GetWeight();

		float __output;
		__output = tmp__output;

		return __output;
	}
} // namespace bs
