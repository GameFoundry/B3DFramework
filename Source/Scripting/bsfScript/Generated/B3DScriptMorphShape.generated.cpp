//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMorphShape.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptMorphShape::ScriptMorphShape(const TShared<MorphShape>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMorphShape::~ScriptMorphShape()
	{
		UnregisterEvents();
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
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MorphShape*>(self->GetNativeObject())->GetName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	float ScriptMorphShape::InternalGetWeight(ScriptMorphShape* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MorphShape*>(self->GetNativeObject())->GetWeight();

		float __output;
		__output = tmp__output;

		return __output;
	}
}
