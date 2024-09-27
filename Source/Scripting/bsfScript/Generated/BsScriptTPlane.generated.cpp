//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTPlane.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace bs
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
