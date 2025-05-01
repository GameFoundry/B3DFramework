//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBone.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCBone.h"

namespace bs
{
	ScriptBone::ScriptBone(const GameObjectHandle<CBone>& nativeObject)
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
		static_cast<CBone*>(self->GetNativeObject())->SetBoneName(tmpname);
	}

	MonoString* ScriptBone::InternalGetBoneName(ScriptBone* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<CBone*>(self->GetNativeObject())->GetBoneName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}
}
