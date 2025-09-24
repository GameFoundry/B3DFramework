//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCapsuleColliderShapeInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptCapsuleColliderShapeInformation::ScriptCapsuleColliderShapeInformation()
	{ }

	MonoObject* ScriptCapsuleColliderShapeInformation::Box(const CapsuleColliderShapeInformation& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	CapsuleColliderShapeInformation ScriptCapsuleColliderShapeInformation::Unbox(MonoObject* value)
	{
		return *(CapsuleColliderShapeInformation*)MonoUtil::Unbox(value);
	}

}
