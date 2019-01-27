//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Extensions/BsSkeletonEx.h"
#include "Math/BsMatrix4.h"

namespace bs
{
	struct __SkeletonBoneInfoExInterop
	{
		MonoString* name;
		int32_t parent;
		Matrix4 invBindPose;
	};

	class BS_SCR_BE_EXPORT ScriptSkeletonBoneInfoEx : public ScriptObject<ScriptSkeletonBoneInfoEx>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "BoneInfo")

		static MonoObject* box(const __SkeletonBoneInfoExInterop& value);
		static __SkeletonBoneInfoExInterop unbox(MonoObject* value);
		static SkeletonBoneInfoEx fromInterop(const __SkeletonBoneInfoExInterop& value);
		static __SkeletonBoneInfoExInterop toInterop(const SkeletonBoneInfoEx& value);

	private:
		ScriptSkeletonBoneInfoEx(MonoObject* managedInstance);

	};
}
