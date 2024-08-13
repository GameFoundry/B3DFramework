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
		static uint32_t InternalAddProbe(ScriptLightProbeVolume* self, TVector3<float>* position);
		static void InternalSetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* position);
		static void InternalGetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* __output);
		static void InternalRemoveProbe(ScriptLightProbeVolume* self, uint32_t handle);
		static MonoArray* InternalGetProbes(ScriptLightProbeVolume* self);
		static void InternalRenderProbe(ScriptLightProbeVolume* self, uint32_t handle);
		static void InternalRenderProbes(ScriptLightProbeVolume* self);
		static void InternalResize(ScriptLightProbeVolume* self, AABox* volume, TVector3I<int32_t>* cellCount);
		static void InternalClip(ScriptLightProbeVolume* self);
		static void InternalReset(ScriptLightProbeVolume* self);
		static void InternalGetGridVolume(ScriptLightProbeVolume* self, AABox* __output);
		static void InternalGetCellCount(ScriptLightProbeVolume* self, TVector3I<int32_t>* __output);
	};
}
