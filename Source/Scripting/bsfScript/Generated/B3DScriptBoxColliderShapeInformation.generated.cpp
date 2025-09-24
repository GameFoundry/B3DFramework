//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptBoxColliderShapeInformation.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace b3d
{
	ScriptBoxColliderShapeInformation::ScriptBoxColliderShapeInformation()
	{ }

	MonoObject* ScriptBoxColliderShapeInformation::Box(const __BoxColliderShapeInformationInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__BoxColliderShapeInformationInterop ScriptBoxColliderShapeInformation::Unbox(MonoObject* value)
	{
		return *(__BoxColliderShapeInformationInterop*)MonoUtil::Unbox(value);
	}

	BoxColliderShapeInformation ScriptBoxColliderShapeInformation::FromInterop(const __BoxColliderShapeInformationInterop& value)
	{
		BoxColliderShapeInformation output;
		output.Extents = value.Extents;

		return output;
	}

	__BoxColliderShapeInformationInterop ScriptBoxColliderShapeInformation::ToInterop(const BoxColliderShapeInformation& value)
	{
		__BoxColliderShapeInformationInterop output;
		output.Extents = value.Extents;

		return output;
	}

}
