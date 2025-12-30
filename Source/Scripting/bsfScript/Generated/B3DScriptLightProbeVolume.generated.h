//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Utility/Math/B3DVector3I.h"
#include "../../../Foundation/Core/Components/B3DLightProbeVolume.h"
#include "../../../Foundation/Utility/Math/B3DVector3.h"
#include "../../../Foundation/Utility/Math/B3DAABox.h"

namespace b3d { class LightProbeVolume; }
namespace b3d { struct __LightProbeInfoInterop; }
namespace b3d { struct __TAABox_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptLightProbeVolume : public TScriptGameObjectWrapper<LightProbeVolume, ScriptLightProbeVolume>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LightProbeVolume")

		ScriptLightProbeVolume(const TGameObjectHandle<LightProbeVolume>& nativeObject);
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
