//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTVector3.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptVector3::ScriptVector3()
	{ }

	MonoObject* ScriptVector3::Box(const TVector3<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector3<float> ScriptVector3::Unbox(MonoObject* value)
	{
		return *(TVector3<float>*)MonoUtil::Unbox(value);
	}


	ScriptVector3D::ScriptVector3D()
	{ }

	MonoObject* ScriptVector3D::Box(const TVector3<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector3<double> ScriptVector3D::Unbox(MonoObject* value)
	{
		return *(TVector3<double>*)MonoUtil::Unbox(value);
	}

}
