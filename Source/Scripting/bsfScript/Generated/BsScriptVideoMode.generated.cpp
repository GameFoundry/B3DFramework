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

	void ScriptVideoMode::initRuntimeData()
	{ }

	MonoObject*ScriptVideoMode::box(const __VideoModeInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__VideoModeInterop ScriptVideoMode::unbox(MonoObject* value)
	{
		return *(__VideoModeInterop*)MonoUtil::unbox(value);
	}

	VideoMode ScriptVideoMode::fromInterop(const __VideoModeInterop& value)
	{
		VideoMode output;
		output.width = value.width;
		output.height = value.height;
		output.refreshRate = value.refreshRate;
		output.outputIdx = value.outputIdx;
		output.isCustom = value.isCustom;

		return output;
	}

	__VideoModeInterop ScriptVideoMode::toInterop(const VideoMode& value)
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
