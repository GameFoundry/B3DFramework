//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DSkeleton.h"
#include "../Extensions/B3DSkeletonEx.h"

namespace b3d { struct __SkeletonBoneInfoExInterop; }
namespace b3d { class SkeletonEx; }
namespace b3d { class Skeleton; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptSkeleton : public TScriptReflectableWrapper<Skeleton, ScriptSkeleton>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Skeleton")

		ScriptSkeleton(const TShared<Skeleton>& nativeObject);
		~ScriptSkeleton();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static uint32_t InternalGetBoneCount(ScriptSkeleton* self);
		static void InternalGetBoneInfo(ScriptSkeleton* self, int32_t boneIdx, __SkeletonBoneInfoExInterop* __output);
	};
}
