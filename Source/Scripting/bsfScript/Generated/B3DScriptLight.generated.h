//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Foundation/Core/Components/B3DLight.h"
#include "../../../Foundation/Utility/Image/B3DColor.h"
#include "Math/B3DDegree.h"
#include "../../../Foundation/Utility/Math/B3DSphere.h"

namespace b3d { class Light; }
namespace b3d { struct __TSphere_float_Interop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptLight : public TScriptGameObjectWrapper<Light, ScriptLight>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Light")

		ScriptLight(const TGameObjectHandle<Light>& nativeObject);
		~ScriptLight();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetType(ScriptLight* self, LightType type);
		static LightType InternalGetType(ScriptLight* self);
		static void InternalSetCastsShadow(ScriptLight* self, bool castsShadow);
		static bool InternalGetCastsShadow(ScriptLight* self);
		static void InternalSetShadowBias(ScriptLight* self, float bias);
		static float InternalGetShadowBias(ScriptLight* self);
		static void InternalSetColor(ScriptLight* self, Color* color);
		static void InternalGetColor(ScriptLight* self, Color* __output);
		static void InternalSetAttenuationRadius(ScriptLight* self, float radius);
		static float InternalGetAttenuationRadius(ScriptLight* self);
		static void InternalSetSourceRadius(ScriptLight* self, float radius);
		static float InternalGetSourceRadius(ScriptLight* self);
		static void InternalSetUseAutoAttenuation(ScriptLight* self, bool enabled);
		static bool InternalGetUseAutoAttenuation(ScriptLight* self);
		static void InternalSetIntensity(ScriptLight* self, float intensity);
		static float InternalGetIntensity(ScriptLight* self);
		static void InternalSetSpotAngle(ScriptLight* self, TDegree<float>* spotAngle);
		static void InternalGetSpotAngle(ScriptLight* self, TDegree<float>* __output);
		static void InternalSetSpotFalloffAngle(ScriptLight* self, TDegree<float>* spotFallofAngle);
		static void InternalGetSpotFalloffAngle(ScriptLight* self, TDegree<float>* __output);
		static void InternalGetBounds(ScriptLight* self, __TSphere_float_Interop* __output);
	};
}
