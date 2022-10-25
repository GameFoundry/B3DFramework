//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptHingeJointDrive.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptHingeJointDrive::ScriptHingeJointDrive(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptHingeJointDrive::InitRuntimeData()
	{}

	MonoObject* ScriptHingeJointDrive::Box(const HingeJointDrive& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	HingeJointDrive ScriptHingeJointDrive::Unbox(MonoObject* value)
	{
		return *(HingeJointDrive*)MonoUtil::Unbox(value);
	}

} // namespace bs
