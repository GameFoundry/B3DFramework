//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVECTOR_FIELD_DESC.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"
#include "BsScriptTAABox.generated.h"

namespace b3d
{
	ScriptVectorFieldOptions::ScriptVectorFieldOptions()
	{ }

	MonoObject* ScriptVectorFieldOptions::Box(const __VECTOR_FIELD_DESCInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__VECTOR_FIELD_DESCInterop ScriptVectorFieldOptions::Unbox(MonoObject* value)
	{
		return *(__VECTOR_FIELD_DESCInterop*)MonoUtil::Unbox(value);
	}

	VECTOR_FIELD_DESC ScriptVectorFieldOptions::FromInterop(const __VECTOR_FIELD_DESCInterop& value)
	{
		VECTOR_FIELD_DESC output;
		output.CountX = value.CountX;
		output.CountY = value.CountY;
		output.CountZ = value.CountZ;
		TAABox<float> tmpBounds;
		tmpBounds = ScriptAABox::FromInterop(value.Bounds);
		output.Bounds = tmpBounds;

		return output;
	}

	__VECTOR_FIELD_DESCInterop ScriptVectorFieldOptions::ToInterop(const VECTOR_FIELD_DESC& value)
	{
		__VECTOR_FIELD_DESCInterop output;
		output.CountX = value.CountX;
		output.CountY = value.CountY;
		output.CountZ = value.CountZ;
		__TAABox_float_Interop tmpBounds;
		tmpBounds = ScriptAABox::ToInterop(value.Bounds);
		output.Bounds = tmpBounds;

		return output;
	}

}
