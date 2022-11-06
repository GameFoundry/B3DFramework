//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSkeleton.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptSkeletonBoneInfoEx.generated.h"
#include "../Extensions/BsSkeletonEx.h"

namespace bs
{
	ScriptSkeleton::ScriptSkeleton(MonoObject* managedInstance, const SPtr<Skeleton>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptSkeleton::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_GetNumBones", (void*)&ScriptSkeleton::InternalGetNumBones);
		metaData.ScriptClass->AddInternalCall("Internal_GetBoneInfo", (void*)&ScriptSkeleton::InternalGetBoneInfo);

	}

	MonoObject* ScriptSkeleton::Create(const SPtr<Skeleton>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptSkeleton>()) ScriptSkeleton(managedInstance, value);
		return managedInstance;
	}
	uint32_t ScriptSkeleton::InternalGetNumBones(ScriptSkeleton* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->GetNumBones();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSkeleton::InternalGetBoneInfo(ScriptSkeleton* thisPtr, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output)
	{
		SkeletonBoneInfoEx tmp__output;
		tmp__output = SkeletonEx::GetBoneInfo(thisPtr->GetInternal(), boneIdx);

		__SkeletonBoneInfoExInterop interop__output;
		interop__output = ScriptSkeletonBoneInfoEx::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptSkeletonBoneInfoEx::GetMetaData()->ScriptClass->GetInternalClassInternal());
	}
}
