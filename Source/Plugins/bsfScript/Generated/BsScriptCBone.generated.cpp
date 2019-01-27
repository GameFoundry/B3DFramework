//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCBone.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../../../Foundation/bsfCore/Components/BsCBone.h"

namespace bs
{
	ScriptCBone::ScriptCBone(MonoObject* managedInstance, const GameObjectHandle<CBone>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCBone::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setBoneName", (void*)&ScriptCBone::Internal_setBoneName);
		metaData.scriptClass->addInternalCall("Internal_getBoneName", (void*)&ScriptCBone::Internal_getBoneName);

	}

	void ScriptCBone::Internal_setBoneName(ScriptCBone* thisPtr, MonoString* name)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		thisPtr->getHandle()->setBoneName(tmpname);
	}

	MonoString* ScriptCBone::Internal_getBoneName(ScriptCBone* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->getHandle()->getBoneName();

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}
}
