//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptColorGradientKey.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Image/BsColor.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptColorGradientKey::ScriptColorGradientKey(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptColorGradientKey::InitRuntimeData()
	{ }

	MonoObject*ScriptColorGradientKey::Box(const __ColorGradientKeyInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
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
