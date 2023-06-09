//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3I.h"
#include "../../../Foundation/bsfCore/Renderer/BsLightProbeVolume.h"
#include "Math/BsVector3.h"
#include "Math/BsAABox.h"

namespace bs { class CLightProbeVolume; }
namespace bs { struct __LightProbeInfoInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptLightProbeVolume : public TScriptComponent<ScriptLightProbeVolume, CLightProbeVolume>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "LightProbeVolume")

		ScriptLightProbeVolume(MonoObject* managedInstance, const GameObjectHandle<CLightProbeVolume>& value);

	private:
		static uint32_t InternalAddProbe(ScriptLightProbeVolume* thisPtr, TVector3<float>* position);
		static void InternalSetProbePosition(ScriptLightProbeVolume* thisPtr, uint32_t handle, TVector3<float>* position);
		static void InternalGetProbePosition(ScriptLightProbeVolume* thisPtr, uint32_t handle, TVector3<float>* __output);
		static void InternalRemoveProbe(ScriptLightProbeVolume* thisPtr, uint32_t handle);
		static MonoArray* InternalGetProbes(ScriptLightProbeVolume* thisPtr);
		static void InternalRenderProbe(ScriptLightProbeVolume* thisPtr, uint32_t handle);
		static void InternalRenderProbes(ScriptLightProbeVolume* thisPtr);
		static void InternalResize(ScriptLightProbeVolume* thisPtr, AABox* volume, TVector3I<int32_t>* cellCount);
		static void InternalClip(ScriptLightProbeVolume* thisPtr);
		static void InternalReset(ScriptLightProbeVolume* thisPtr);
		static void InternalGetGridVolume(ScriptLightProbeVolume* thisPtr, AABox* __output);
		static void InternalGetCellCount(ScriptLightProbeVolume* thisPtr, TVector3I<int32_t>* __output);
	};
}
