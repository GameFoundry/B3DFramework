//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptLightProbeInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "BsScriptTVector3.generated.h"

namespace b3d
{
	ScriptLightProbeInfo::ScriptLightProbeInfo()
	{ }

	MonoObject* ScriptLightProbeInfo::Box(const __LightProbeInfoInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
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

}
