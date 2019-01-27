//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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

	void ScriptColorGradientKey::initRuntimeData()
	{ }

	MonoObject*ScriptColorGradientKey::box(const __ColorGradientKeyInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__ColorGradientKeyInterop ScriptColorGradientKey::unbox(MonoObject* value)
	{
		return *(__ColorGradientKeyInterop*)MonoUtil::unbox(value);
	}

	ColorGradientKey ScriptColorGradientKey::fromInterop(const __ColorGradientKeyInterop& value)
	{
		ColorGradientKey output;
		output.color = value.color;
		output.time = value.time;

		return output;
	}

	__ColorGradientKeyInterop ScriptColorGradientKey::toInterop(const ColorGradientKey& value)
	{
		__ColorGradientKeyInterop output;
		output.color = value.color;
		output.time = value.time;

		return output;
	}

}
