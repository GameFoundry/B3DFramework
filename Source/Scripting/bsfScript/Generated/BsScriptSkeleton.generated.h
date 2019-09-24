//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Animation/BsSkeleton.h"
#include "../Extensions/BsSkeletonEx.h"

namespace bs { class Skeleton; }
namespace bs { class SkeletonEx; }
namespace bs { struct __SkeletonBoneInfoExInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptSkeleton : public TScriptReflectable<ScriptSkeleton, Skeleton>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Skeleton")

		ScriptSkeleton(MonoObject* managedInstance, const SPtr<Skeleton>& value);

		static MonoObject* create(const SPtr<Skeleton>& value);

	private:
		static uint32_t Internal_getNumBones(ScriptSkeleton* thisPtr);
		static void Internal_getBoneInfo(ScriptSkeleton* thisPtr, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output);
	};
}
