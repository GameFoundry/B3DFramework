//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSkeletonBoneInfoEx.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptSkeletonBoneInfoEx::ScriptSkeletonBoneInfoEx(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptSkeletonBoneInfoEx::initRuntimeData()
	{ }

	MonoObject*ScriptSkeletonBoneInfoEx::box(const __SkeletonBoneInfoExInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__SkeletonBoneInfoExInterop ScriptSkeletonBoneInfoEx::unbox(MonoObject* value)
	{
		return *(__SkeletonBoneInfoExInterop*)MonoUtil::unbox(value);
	}

	SkeletonBoneInfoEx ScriptSkeletonBoneInfoEx::fromInterop(const __SkeletonBoneInfoExInterop& value)
	{
		SkeletonBoneInfoEx output;
		String tmpname;
		tmpname = MonoUtil::monoToString(value.name);
		output.name = tmpname;
		output.parent = value.parent;
		output.invBindPose = value.invBindPose;

		return output;
	}

	__SkeletonBoneInfoExInterop ScriptSkeletonBoneInfoEx::toInterop(const SkeletonBoneInfoEx& value)
	{
		__SkeletonBoneInfoExInterop output;
		MonoString* tmpname;
		tmpname = MonoUtil::stringToMono(value.name);
		output.name = tmpname;
		output.parent = value.parent;
		output.invBindPose = value.invBindPose;

		return output;
	}

}
