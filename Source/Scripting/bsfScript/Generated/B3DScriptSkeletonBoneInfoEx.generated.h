//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../Extensions/B3DSkeletonEx.h"
#include "Math/B3DMatrix4.h"

namespace b3d
{
	struct __SkeletonBoneInfoExInterop
	{
		MonoString* Name;
		int32_t Parent;
		TMatrix4<float> InvBindPose;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoneInfo : public TScriptTypeDefinition<ScriptBoneInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "BoneInfo")

		static MonoObject* Box(const __SkeletonBoneInfoExInterop& value);
		static __SkeletonBoneInfoExInterop Unbox(MonoObject* value);
		static SkeletonBoneInfoEx FromInterop(const __SkeletonBoneInfoExInterop& value);
		static __SkeletonBoneInfoExInterop ToInterop(const SkeletonBoneInfoEx& value);

	private:
		ScriptBoneInfo();

	};
}
