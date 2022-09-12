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
		:ScriptObject(managedInstance)
	{ }

	void ScriptVideoMode::InitRuntimeData()
	{ }

	MonoObject*ScriptVideoMode::box(const __VideoModeInterop& value)
	{
		return MonoUtil::Box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__VideoModeInterop ScriptVideoMode::Unbox(MonoObject* value)
	{
		return *(__VideoModeInterop*)MonoUtil::unbox(value);
	}

	VideoMode ScriptVideoMode::FromInterop(const __VideoModeInterop& value)
	{
		VideoMode output;
		output.width = value.width;
		output.height = value.height;
		output.refreshRate = value.refreshRate;
		output.outputIdx = value.outputIdx;
		output.isCustom = value.isCustom;

		return output;
	}

	__VideoModeInterop ScriptVideoMode::ToInterop(const VideoMode& value)
	{
		__VideoModeInterop output;
		output.width = value.width;
		output.height = value.height;
		output.refreshRate = value.refreshRate;
		output.outputIdx = value.outputIdx;
		output.isCustom = value.isCustom;

		return output;
	}

#endif
}
