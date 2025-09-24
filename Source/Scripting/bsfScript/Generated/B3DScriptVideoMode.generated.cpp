//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptVideoMode.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptVideoMode::ScriptVideoMode()
	{ }

	MonoObject* ScriptVideoMode::Box(const __VideoModeInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__VideoModeInterop ScriptVideoMode::Unbox(MonoObject* value)
	{
		return *(__VideoModeInterop*)MonoUtil::Unbox(value);
	}

	VideoMode ScriptVideoMode::FromInterop(const __VideoModeInterop& value)
	{
		VideoMode output;
		output.Width = value.Width;
		output.Height = value.Height;
		output.RefreshRate = value.RefreshRate;
		output.OutputIdx = value.OutputIdx;
		output.IsCustom = value.IsCustom;

		return output;
	}

	__VideoModeInterop ScriptVideoMode::ToInterop(const VideoMode& value)
	{
		__VideoModeInterop output;
		output.Width = value.Width;
		output.Height = value.Height;
		output.RefreshRate = value.RefreshRate;
		output.OutputIdx = value.OutputIdx;
		output.IsCustom = value.IsCustom;

		return output;
	}

#endif
}
