//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTQuaternion.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptQuaternion::ScriptQuaternion()
	{ }

	MonoObject* ScriptQuaternion::Box(const TQuaternion<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TQuaternion<float> ScriptQuaternion::Unbox(MonoObject* value)
	{
		return *(TQuaternion<float>*)MonoUtil::Unbox(value);
	}


	ScriptQuaternionD::ScriptQuaternionD()
	{ }

	MonoObject* ScriptQuaternionD::Box(const TQuaternion<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TQuaternion<double> ScriptQuaternionD::Unbox(MonoObject* value)
	{
		return *(TQuaternion<double>*)MonoUtil::Unbox(value);
	}

}
