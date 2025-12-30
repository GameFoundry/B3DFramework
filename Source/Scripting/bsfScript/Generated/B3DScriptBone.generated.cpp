//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptBone.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Core/Components/B3DBone.h"

namespace b3d
{
	ScriptBone::ScriptBone(const TGameObjectHandle<Bone>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptBone::~ScriptBone()
	{
		UnregisterEvents();
	}

	void ScriptBone::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBoneName", (void*)&ScriptBone::InternalSetBoneName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBoneName", (void*)&ScriptBone::InternalGetBoneName);

	}

	MonoObject* ScriptBone::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptBone::InternalSetBoneName(ScriptBone* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<Bone*>(self->GetNativeObject())->SetBoneName(tmpname);
	}

	MonoString* ScriptBone::InternalGetBoneName(ScriptBone* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Bone*>(self->GetNativeObject())->GetBoneName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}
}
