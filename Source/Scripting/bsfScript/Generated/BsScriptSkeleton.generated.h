//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsSkeleton.h"
#include "../Extensions/BsSkeletonEx.h"

namespace bs { class Skeleton; }
namespace bs { class SkeletonEx; }
namespace bs { struct __SkeletonBoneInfoExInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSkeleton : public TScriptReflectableWrapper<Skeleton, ScriptSkeleton>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Skeleton")

		ScriptSkeleton(const SPtr<Skeleton>& nativeObject);
		~ScriptSkeleton();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static uint32_t InternalGetNumBones(ScriptSkeleton* self);
		static void InternalGetBoneInfo(ScriptSkeleton* self, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output);
	};
}
