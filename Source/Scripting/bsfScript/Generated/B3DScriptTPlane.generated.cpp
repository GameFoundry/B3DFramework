//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTPlane.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptPlane::ScriptPlane()
	{ }

	MonoObject* ScriptPlane::Box(const __TPlane_float_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TPlane_float_Interop ScriptPlane::Unbox(MonoObject* value)
	{
		return *(__TPlane_float_Interop*)MonoUtil::Unbox(value);
	}

	TPlane<float> ScriptPlane::FromInterop(const __TPlane_float_Interop& value)
	{
		TPlane<float> output;
		output.Normal = value.Normal;
		output.D = value.D;

		return output;
	}

	__TPlane_float_Interop ScriptPlane::ToInterop(const TPlane<float>& value)
	{
		__TPlane_float_Interop output;
		output.Normal = value.Normal;
		output.D = value.D;

		return output;
	}


	ScriptPlaneD::ScriptPlaneD()
	{ }

	MonoObject* ScriptPlaneD::Box(const __TPlane_double_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TPlane_double_Interop ScriptPlaneD::Unbox(MonoObject* value)
	{
		return *(__TPlane_double_Interop*)MonoUtil::Unbox(value);
	}

	TPlane<double> ScriptPlaneD::FromInterop(const __TPlane_double_Interop& value)
	{
		TPlane<double> output;
		output.Normal = value.Normal;
		output.D = value.D;

		return output;
	}

	__TPlane_double_Interop ScriptPlaneD::ToInterop(const TPlane<double>& value)
	{
		__TPlane_double_Interop output;
		output.Normal = value.Normal;
		output.D = value.D;

		return output;
	}

}
