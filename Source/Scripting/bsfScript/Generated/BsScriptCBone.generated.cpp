//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBone.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCBone.h"

namespace bs
{
	ScriptCBone::ScriptCBone(MonoObject* managedInstance, const GameObjectHandle<CBone>& value)
		: TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCBone::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetBoneName", (void*)&ScriptCBone::InternalSetBoneName);
		metaData.ScriptClass->AddInternalCall("Internal_GetBoneName", (void*)&ScriptCBone::InternalGetBoneName);
	}

	void ScriptCBone::InternalSetBoneName(ScriptCBone* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		thisPtr->GetHandle()->SetBoneName(tmpname);
	}

	MonoString* ScriptCBone::InternalGetBoneName(ScriptCBone* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBoneName();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}
} // namespace bs
