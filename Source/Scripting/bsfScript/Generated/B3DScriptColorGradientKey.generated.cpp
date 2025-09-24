//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptColorGradientKey.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"
#include "B3DScriptColor.generated.h"

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
