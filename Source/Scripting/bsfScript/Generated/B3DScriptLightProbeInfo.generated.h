//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Components/B3DLightProbeVolume.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"

namespace b3d
{
	struct __LightProbeInfoInterop
	{
		uint32_t Handle;
		TVector3<float> Position;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptLightProbeInfo : public TScriptTypeDefinition<ScriptLightProbeInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LightProbeInfo")

		static MonoObject* Box(const __LightProbeInfoInterop& value);
		static __LightProbeInfoInterop Unbox(MonoObject* value);
		static LightProbeInfo FromInterop(const __LightProbeInfoInterop& value);
		static __LightProbeInfoInterop ToInterop(const LightProbeInfo& value);

	private:
		ScriptLightProbeInfo();

	};
}
