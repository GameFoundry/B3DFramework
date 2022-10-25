//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptVideoMode.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptVideoMode::ScriptVideoMode(MonoObject* managedInstance)
		: ScriptObject(managedInstance)
	{}

	void ScriptVideoMode::InitRuntimeData()
	{}

	MonoObject* ScriptVideoMode::Box(const __VideoModeInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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
} // namespace bs
