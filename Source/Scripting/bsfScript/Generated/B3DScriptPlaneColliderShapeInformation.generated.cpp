//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptPlaneColliderShapeInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptPlaneColliderShapeInformation::ScriptPlaneColliderShapeInformation()
	{ }

	MonoObject* ScriptPlaneColliderShapeInformation::Box(const PlaneColliderShapeInformation& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	PlaneColliderShapeInformation ScriptPlaneColliderShapeInformation::Unbox(MonoObject* value)
	{
		return *(PlaneColliderShapeInformation*)MonoUtil::Unbox(value);
	}

}
