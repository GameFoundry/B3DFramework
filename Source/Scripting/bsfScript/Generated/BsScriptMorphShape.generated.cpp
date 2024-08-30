//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMorphShape.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptMorphShape::ScriptMorphShape(const SPtr<MorphShape>& nativeObject, MonoObject* scriptObject)
		:TScriptReflectableWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptMorphShape::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptMorphShape::InternalGetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetWeight", (void*)&ScriptMorphShape::InternalGetWeight);

	}

	MonoObject* ScriptMorphShape::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	MonoString* ScriptMorphShape::InternalGetName(ScriptMorphShape* self)
	{
		String tmp__output;
		tmp__output = static_cast<MorphShape*>(self->GetNativeObject())->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	float ScriptMorphShape::InternalGetWeight(ScriptMorphShape* self)
	{
		float tmp__output;
		tmp__output = static_cast<MorphShape*>(self->GetNativeObject())->GetWeight();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
