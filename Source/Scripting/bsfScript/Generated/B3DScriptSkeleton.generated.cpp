//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSkeleton.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptSkeletonBoneInfoEx.generated.h"
#include "../Extensions/BsSkeletonEx.h"

namespace b3d
{
	ScriptSkeleton::ScriptSkeleton(const SPtr<Skeleton>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSkeleton::~ScriptSkeleton()
	{
		UnregisterEvents();
	}

	void ScriptSkeleton::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBoneCount", (void*)&ScriptSkeleton::InternalGetBoneCount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBoneInfo", (void*)&ScriptSkeleton::InternalGetBoneInfo);

	}

	MonoObject* ScriptSkeleton::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	uint32_t ScriptSkeleton::InternalGetBoneCount(ScriptSkeleton* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Skeleton*>(self->GetNativeObject())->GetBoneCount();

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSkeleton::InternalGetBoneInfo(ScriptSkeleton* self, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		SkeletonBoneInfoEx tmp__output;
		tmp__output = SkeletonEx::GetBoneInfo(std::static_pointer_cast<Skeleton>(self->GetBaseNativeObjectAsShared()), boneIdx);

		__SkeletonBoneInfoExInterop interop__output;
		interop__output = ScriptBoneInfo::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptBoneInfo::GetMetaData()->ScriptClass->GetInternalClass());
	}
}
