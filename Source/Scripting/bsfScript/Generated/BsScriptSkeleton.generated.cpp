//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptSkeleton::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_getNumBones", (void*)&ScriptSkeleton::Internal_getNumBones);
		metaData.scriptClass->addInternalCall("Internal_getBoneInfo", (void*)&ScriptSkeleton::Internal_getBoneInfo);

	}

	MonoObject* ScriptSkeleton::create(const SPtr<Skeleton>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
		new (bs_alloc<ScriptSkeleton>()) ScriptSkeleton(managedInstance, value);
		return managedInstance;
	}
	uint32_t ScriptSkeleton::Internal_getNumBones(ScriptSkeleton* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->getNumBones();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSkeleton::Internal_getBoneInfo(ScriptSkeleton* thisPtr, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output)
	{
		SkeletonBoneInfoEx tmp__output;
		tmp__output = SkeletonEx::getBoneInfo(thisPtr->getInternal(), boneIdx);

		__SkeletonBoneInfoExInterop interop__output;
		interop__output = ScriptSkeletonBoneInfoEx::toInterop(tmp__output);
		MonoUtil::valueCopy(__output, &interop__output, ScriptSkeletonBoneInfoEx::getMetaData()->scriptClass->_getInternalClass());
	}
}
