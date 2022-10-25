//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBoneWeight.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptBoneWeight::ScriptBoneWeight(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptBoneWeight::InitRuntimeData()
	{}

	MonoObject* ScriptBoneWeight::Box(const BoneWeight& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	BoneWeight ScriptBoneWeight::Unbox(MonoObject* value)
	{
		return *(BoneWeight*)MonoUtil::Unbox(value);
	}

} // namespace bs
