//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Renderer/BsLightProbeVolume.h"
#include "Math/BsVector3.h"

namespace bs
{
	struct __LightProbeInfoInterop
	{
		uint32_t Handle;
		Vector3 Position;
	};

	class BS_SCR_BE_EXPORT ScriptLightProbeInfo : public ScriptObject<ScriptLightProbeInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "LightProbeInfo")

		static MonoObject* Box(const __LightProbeInfoInterop& value);
		static __LightProbeInfoInterop Unbox(MonoObject* value);
		static LightProbeInfo FromInterop(const __LightProbeInfoInterop& value);
		static __LightProbeInfoInterop ToInterop(const LightProbeInfo& value);

	private:
		ScriptLightProbeInfo(MonoObject* managedInstance);
	};
} // namespace bs
