//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObjectWrapper.h"
#include "../Extensions/BsSkeletonEx.h"
#include "Math/BsMatrix4.h"

namespace bs
{
	struct __SkeletonBoneInfoExInterop
	{
		MonoString* Name;
		int32_t Parent;
		Matrix4 InvBindPose;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptBoneInfo : public TScriptStructWrapper<ScriptBoneInfo>
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
