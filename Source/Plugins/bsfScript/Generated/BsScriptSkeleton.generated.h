//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../Extensions/BsSkeletonEx.h"

namespace bs
{
	class Skeleton;
	class SkeletonEx;
	struct __SkeletonBoneInfoExInterop;

	class BS_SCR_BE_EXPORT ScriptSkeleton : public ScriptObject<ScriptSkeleton>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Skeleton")

		ScriptSkeleton(MonoObject* managedInstance, const SPtr<Skeleton>& value);

		SPtr<Skeleton> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<Skeleton>& value);

	private:
		SPtr<Skeleton> mInternal;

		static uint32_t Internal_getNumBones(ScriptSkeleton* thisPtr);
		static void Internal_getBoneInfo(ScriptSkeleton* thisPtr, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output);
	};
}
