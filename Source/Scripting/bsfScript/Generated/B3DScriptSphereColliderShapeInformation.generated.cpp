//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptSphereColliderShapeInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptSphereColliderShapeInformation::ScriptSphereColliderShapeInformation()
	{ }

	MonoObject* ScriptSphereColliderShapeInformation::Box(const SphereColliderShapeInformation& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	SphereColliderShapeInformation ScriptSphereColliderShapeInformation::Unbox(MonoObject* value)
	{
		return *(SphereColliderShapeInformation*)MonoUtil::Unbox(value);
	}

}
