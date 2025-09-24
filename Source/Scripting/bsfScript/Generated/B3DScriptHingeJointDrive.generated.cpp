//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptHingeJointDrive.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptHingeJointDrive::ScriptHingeJointDrive()
	{ }

	MonoObject* ScriptHingeJointDrive::Box(const HingeJointDrive& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	HingeJointDrive ScriptHingeJointDrive::Unbox(MonoObject* value)
	{
		return *(HingeJointDrive*)MonoUtil::Unbox(value);
	}

}
