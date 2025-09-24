//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTSphere.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptSphere::ScriptSphere()
	{ }

	MonoObject* ScriptSphere::Box(const __TSphere_float_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TSphere_float_Interop ScriptSphere::Unbox(MonoObject* value)
	{
		return *(__TSphere_float_Interop*)MonoUtil::Unbox(value);
	}

	TSphere<float> ScriptSphere::FromInterop(const __TSphere_float_Interop& value)
	{
		TSphere<float> output;
		output.Radius = value.Radius;
		output.Center = value.Center;

		return output;
	}

	__TSphere_float_Interop ScriptSphere::ToInterop(const TSphere<float>& value)
	{
		__TSphere_float_Interop output;
		output.Radius = value.Radius;
		output.Center = value.Center;

		return output;
	}


	ScriptSphereD::ScriptSphereD()
	{ }

	MonoObject* ScriptSphereD::Box(const __TSphere_double_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TSphere_double_Interop ScriptSphereD::Unbox(MonoObject* value)
	{
		return *(__TSphere_double_Interop*)MonoUtil::Unbox(value);
	}

	TSphere<double> ScriptSphereD::FromInterop(const __TSphere_double_Interop& value)
	{
		TSphere<double> output;
		output.Radius = value.Radius;
		output.Center = value.Center;

		return output;
	}

	__TSphere_double_Interop ScriptSphereD::ToInterop(const TSphere<double>& value)
	{
		__TSphere_double_Interop output;
		output.Radius = value.Radius;
		output.Center = value.Center;

		return output;
	}

}
