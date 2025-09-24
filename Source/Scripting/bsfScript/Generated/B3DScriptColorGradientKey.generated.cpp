//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradientKey.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Image/BsColor.h"
#include "BsScriptColor.generated.h"

namespace b3d
{
	ScriptColorGradientKey::ScriptColorGradientKey()
	{ }

	MonoObject* ScriptColorGradientKey::Box(const __ColorGradientKeyInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__ColorGradientKeyInterop ScriptColorGradientKey::Unbox(MonoObject* value)
	{
		return *(__ColorGradientKeyInterop*)MonoUtil::Unbox(value);
	}

	ColorGradientKey ScriptColorGradientKey::FromInterop(const __ColorGradientKeyInterop& value)
	{
		ColorGradientKey output;
		output.Color = value.Color;
		output.Time = value.Time;

		return output;
	}

	__ColorGradientKeyInterop ScriptColorGradientKey::ToInterop(const ColorGradientKey& value)
	{
		__ColorGradientKeyInterop output;
		output.Color = value.Color;
		output.Time = value.Time;

		return output;
	}

}
