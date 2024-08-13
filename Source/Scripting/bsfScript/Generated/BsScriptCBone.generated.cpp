//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBone.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCBone.h"

namespace bs
{
	ScriptBone::ScriptBone(MonoObject* managedInstance, const GameObjectHandle<CBone>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptBone::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetBoneName", (void*)&ScriptBone::InternalSetBoneName);
		metaData.ScriptClass->AddInternalCall("Internal_GetBoneName", (void*)&ScriptBone::InternalGetBoneName);

	}

	void ScriptBone::InternalSetBoneName(ScriptBone* self, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		self->GetHandle()->SetBoneName(tmpname);
	}

	MonoString* ScriptBone::InternalGetBoneName(ScriptBone* self)
	{
		String tmp__output;
		tmp__output = self->GetHandle()->GetBoneName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}
}
