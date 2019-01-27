//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLightProbeInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "Math/BsVector3.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptLightProbeInfo::ScriptLightProbeInfo(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptLightProbeInfo::initRuntimeData()
	{ }

	MonoObject*ScriptLightProbeInfo::box(const __LightProbeInfoInterop& value)
	{
		return MonoUtil::box(metaData.scriptClass->_getInternalClass(), (void*)&value);
	}

	__LightProbeInfoInterop ScriptLightProbeInfo::unbox(MonoObject* value)
	{
		return *(__LightProbeInfoInterop*)MonoUtil::unbox(value);
	}

	LightProbeInfo ScriptLightProbeInfo::fromInterop(const __LightProbeInfoInterop& value)
	{
		LightProbeInfo output;
		output.handle = value.handle;
		output.position = value.position;

		return output;
	}

	__LightProbeInfoInterop ScriptLightProbeInfo::toInterop(const LightProbeInfo& value)
	{
		__LightProbeInfoInterop output;
		output.handle = value.handle;
		output.position = value.position;

		return output;
	}

}
