//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
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

	class BS_SCR_BE_EXPORT ScriptSkeletonBoneInfoEx : public ScriptObject<ScriptSkeletonBoneInfoEx>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "BoneInfo")

		static MonoObject* Box(const __SkeletonBoneInfoExInterop& value);
		static __SkeletonBoneInfoExInterop Unbox(MonoObject* value);
		static SkeletonBoneInfoEx FromInterop(const __SkeletonBoneInfoExInterop& value);
		static __SkeletonBoneInfoExInterop ToInterop(const SkeletonBoneInfoEx& value);

	private:
		ScriptSkeletonBoneInfoEx(MonoObject* managedInstance);
	};
} // namespace bs
