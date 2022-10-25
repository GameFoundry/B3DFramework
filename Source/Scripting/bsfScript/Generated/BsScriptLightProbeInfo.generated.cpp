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
		: ScriptObject(managedInstance)
	{}

	void ScriptLightProbeInfo::InitRuntimeData()
	{}

	MonoObject* ScriptLightProbeInfo::Box(const __LightProbeInfoInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__LightProbeInfoInterop ScriptLightProbeInfo::Unbox(MonoObject* value)
	{
		return *(__LightProbeInfoInterop*)MonoUtil::Unbox(value);
	}

	LightProbeInfo ScriptLightProbeInfo::FromInterop(const __LightProbeInfoInterop& value)
	{
		LightProbeInfo output;
		output.Handle = value.Handle;
		output.Position = value.Position;

		return output;
	}

	__LightProbeInfoInterop ScriptLightProbeInfo::ToInterop(const LightProbeInfo& value)
	{
		__LightProbeInfoInterop output;
		output.Handle = value.Handle;
		output.Position = value.Position;

		return output;
	}

} // namespace bs
