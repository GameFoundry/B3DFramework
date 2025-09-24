//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptTRay.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "B3DScriptTVector3.generated.h"

namespace b3d
{
	ScriptRay::ScriptRay()
	{ }

	MonoObject* ScriptRay::Box(const __TRay_float_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TRay_float_Interop ScriptRay::Unbox(MonoObject* value)
	{
		return *(__TRay_float_Interop*)MonoUtil::Unbox(value);
	}

	TRay<float> ScriptRay::FromInterop(const __TRay_float_Interop& value)
	{
		TRay<float> output;
		output.Origin = value.Origin;
		output.Direction = value.Direction;

		return output;
	}

	__TRay_float_Interop ScriptRay::ToInterop(const TRay<float>& value)
	{
		__TRay_float_Interop output;
		output.Origin = value.Origin;
		output.Direction = value.Direction;

		return output;
	}


	ScriptRayD::ScriptRayD()
	{ }

	MonoObject* ScriptRayD::Box(const __TRay_double_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TRay_double_Interop ScriptRayD::Unbox(MonoObject* value)
	{
		return *(__TRay_double_Interop*)MonoUtil::Unbox(value);
	}

	TRay<double> ScriptRayD::FromInterop(const __TRay_double_Interop& value)
	{
		TRay<double> output;
		output.Origin = value.Origin;
		output.Direction = value.Direction;

		return output;
	}

	__TRay_double_Interop ScriptRayD::ToInterop(const TRay<double>& value)
	{
		__TRay_double_Interop output;
		output.Origin = value.Origin;
		output.Direction = value.Direction;

		return output;
	}

}
