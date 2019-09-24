//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "Math/BsVector3.h"
#include "../../../Foundation/bsfCore/Renderer/BsLightProbeVolume.h"
#include "Math/BsAABox.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3I.h"

namespace bs { class CLightProbeVolume; }
namespace bs { struct __LightProbeInfoInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCLightProbeVolume : public TScriptComponent<ScriptCLightProbeVolume, CLightProbeVolume>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "LightProbeVolume")

		ScriptCLightProbeVolume(MonoObject* managedInstance, const GameObjectHandle<CLightProbeVolume>& value);

	private:
		static uint32_t Internal_addProbe(ScriptCLightProbeVolume* thisPtr, Vector3* position);
		static void Internal_setProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* position);
		static void Internal_getProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* __output);
		static void Internal_removeProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle);
		static MonoArray* Internal_getProbes(ScriptCLightProbeVolume* thisPtr);
		static void Internal_renderProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle);
		static void Internal_renderProbes(ScriptCLightProbeVolume* thisPtr);
		static void Internal_resize(ScriptCLightProbeVolume* thisPtr, AABox* volume, Vector3I* cellCount);
		static void Internal_clip(ScriptCLightProbeVolume* thisPtr);
		static void Internal_reset(ScriptCLightProbeVolume* thisPtr);
		static void Internal_getGridVolume(ScriptCLightProbeVolume* thisPtr, AABox* __output);
		static void Internal_getCellCount(ScriptCLightProbeVolume* thisPtr, Vector3I* __output);
	};
}
