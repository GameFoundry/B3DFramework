//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3I.h"
#include "../../../Foundation/bsfCore/Components/BsLightProbeVolume.h"
#include "../../../Foundation/bsfUtility/Math/BsVector3.h"
#include "../../../Foundation/bsfUtility/Math/BsAABox.h"

namespace b3d { class LightProbeVolume; }
namespace b3d { struct __LightProbeInfoInterop; }
namespace b3d { struct __TAABox_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptLightProbeVolume : public TScriptGameObjectWrapper<LightProbeVolume, ScriptLightProbeVolume>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LightProbeVolume")

		ScriptLightProbeVolume(const GameObjectHandle<LightProbeVolume>& nativeObject);
		~ScriptLightProbeVolume();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static uint32_t InternalAddProbe(ScriptLightProbeVolume* self, TVector3<float>* position);
		static void InternalRemoveProbe(ScriptLightProbeVolume* self, uint32_t handle);
		static void InternalSetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* position);
		static void InternalGetProbePosition(ScriptLightProbeVolume* self, uint32_t handle, TVector3<float>* __output);
		static MonoArray* InternalGetProbes(ScriptLightProbeVolume* self);
		static void InternalRenderProbe(ScriptLightProbeVolume* self, uint32_t handle);
		static void InternalRenderProbes(ScriptLightProbeVolume* self);
		static void InternalResize(ScriptLightProbeVolume* self, __TAABox_float_Interop* volume, TVector3I<int32_t>* cellCount);
		static void InternalClip(ScriptLightProbeVolume* self);
		static void InternalReset(ScriptLightProbeVolume* self);
		static void InternalGetGridVolume(ScriptLightProbeVolume* self, __TAABox_float_Interop* __output);
		static void InternalGetCellCount(ScriptLightProbeVolume* self, TVector3I<int32_t>* __output);
	};
}
