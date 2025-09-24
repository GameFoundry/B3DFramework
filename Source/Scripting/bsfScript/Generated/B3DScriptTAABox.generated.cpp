//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptTAABox.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace b3d
{
	ScriptAABox::ScriptAABox()
	{ }

	MonoObject* ScriptAABox::Box(const __TAABox_float_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TAABox_float_Interop ScriptAABox::Unbox(MonoObject* value)
	{
		return *(__TAABox_float_Interop*)MonoUtil::Unbox(value);
	}

	TAABox<float> ScriptAABox::FromInterop(const __TAABox_float_Interop& value)
	{
		TAABox<float> output;
		output.Minimum = value.Minimum;
		output.Maximum = value.Maximum;

		return output;
	}

	__TAABox_float_Interop ScriptAABox::ToInterop(const TAABox<float>& value)
	{
		__TAABox_float_Interop output;
		output.Minimum = value.Minimum;
		output.Maximum = value.Maximum;

		return output;
	}


	ScriptAABoxD::ScriptAABoxD()
	{ }

	MonoObject* ScriptAABoxD::Box(const __TAABox_double_Interop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__TAABox_double_Interop ScriptAABoxD::Unbox(MonoObject* value)
	{
		return *(__TAABox_double_Interop*)MonoUtil::Unbox(value);
	}

	TAABox<double> ScriptAABoxD::FromInterop(const __TAABox_double_Interop& value)
	{
		TAABox<double> output;
		output.Minimum = value.Minimum;
		output.Maximum = value.Maximum;

		return output;
	}

	__TAABox_double_Interop ScriptAABoxD::ToInterop(const TAABox<double>& value)
	{
		__TAABox_double_Interop output;
		output.Minimum = value.Minimum;
		output.Maximum = value.Maximum;

		return output;
	}

}
