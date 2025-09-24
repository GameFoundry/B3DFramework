//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTVector4.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptVector4::ScriptVector4()
	{ }

	MonoObject* ScriptVector4::Box(const TVector4<float>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector4<float> ScriptVector4::Unbox(MonoObject* value)
	{
		return *(TVector4<float>*)MonoUtil::Unbox(value);
	}


	ScriptVector4D::ScriptVector4D()
	{ }

	MonoObject* ScriptVector4D::Box(const TVector4<double>& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	TVector4<double> ScriptVector4D::Unbox(MonoObject* value)
	{
		return *(TVector4<double>*)MonoUtil::Unbox(value);
	}

}
