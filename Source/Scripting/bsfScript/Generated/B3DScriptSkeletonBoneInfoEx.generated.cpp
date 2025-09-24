//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSkeletonBoneInfoEx.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptBoneInfo::ScriptBoneInfo()
	{ }

	MonoObject* ScriptBoneInfo::Box(const __SkeletonBoneInfoExInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__SkeletonBoneInfoExInterop ScriptBoneInfo::Unbox(MonoObject* value)
	{
		return *(__SkeletonBoneInfoExInterop*)MonoUtil::Unbox(value);
	}

	SkeletonBoneInfoEx ScriptBoneInfo::FromInterop(const __SkeletonBoneInfoExInterop& value)
	{
		SkeletonBoneInfoEx output;
		String tmpName;
		tmpName = MonoUtil::MonoToString(value.Name);
		output.Name = tmpName;
		output.Parent = value.Parent;
		output.InvBindPose = value.InvBindPose;

		return output;
	}

	__SkeletonBoneInfoExInterop ScriptBoneInfo::ToInterop(const SkeletonBoneInfoEx& value)
	{
		__SkeletonBoneInfoExInterop output;
		MonoString* tmpName;
		tmpName = MonoUtil::StringToMono(value.Name);
		output.Name = tmpName;
		output.Parent = value.Parent;
		output.InvBindPose = value.InvBindPose;

		return output;
	}

}
