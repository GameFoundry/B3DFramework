//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCLightProbeVolume : public TScriptComponent<ScriptCLightProbeVolume, CLightProbeVolume>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "LightProbeVolume")

		ScriptCLightProbeVolume(MonoObject* managedInstance, const GameObjectHandle<CLightProbeVolume>& value);

	private:
		static uint32_t InternalAddProbe(ScriptCLightProbeVolume* thisPtr, Vector3* position);
		static void InternalSetProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* position);
		static void InternalGetProbePosition(ScriptCLightProbeVolume* thisPtr, uint32_t handle, Vector3* __output);
		static void InternalRemoveProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle);
		static MonoArray* InternalGetProbes(ScriptCLightProbeVolume* thisPtr);
		static void InternalRenderProbe(ScriptCLightProbeVolume* thisPtr, uint32_t handle);
		static void InternalRenderProbes(ScriptCLightProbeVolume* thisPtr);
		static void InternalResize(ScriptCLightProbeVolume* thisPtr, AABox* volume, Vector3I* cellCount);
		static void InternalClip(ScriptCLightProbeVolume* thisPtr);
		static void InternalReset(ScriptCLightProbeVolume* thisPtr);
		static void InternalGetGridVolume(ScriptCLightProbeVolume* thisPtr, AABox* __output);
		static void InternalGetCellCount(ScriptCLightProbeVolume* thisPtr, Vector3I* __output);
	};
}
