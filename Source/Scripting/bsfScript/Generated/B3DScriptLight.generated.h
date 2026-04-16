//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"
#include "../../../Engine/Core/Components/B3DLight.h"
#include "../../../Engine/Utility/Image/B3DColor.h"
#include "Math/B3DDegree.h"
#include "../../../Engine/Utility/Math/B3DSphere.h"

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
		static void InternalSetCastsShadow(ScriptLight* self, bool castsShadow);
		static void InternalSetShadowBias(ScriptLight* self, float bias);
		static void InternalSetColor(ScriptLight* self, Color* color);
		static void InternalSetAttenuationRadius(ScriptLight* self, float radius);
		static void InternalSetSourceRadius(ScriptLight* self, float radius);
		static void InternalSetUseAutoAttenuation(ScriptLight* self, bool enabled);
		static void InternalSetIntensity(ScriptLight* self, float intensity);
		static void InternalSetSpotAngle(ScriptLight* self, TDegree<float>* spotAngle);
		static void InternalSetSpotFalloffAngle(ScriptLight* self, TDegree<float>* spotFallofAngle);
		static LightType InternalGetType(ScriptLight* self);
		static bool InternalGetCastsShadow(ScriptLight* self);
		static float InternalGetShadowBias(ScriptLight* self);
		static void InternalGetColor(ScriptLight* self, Color* __output);
		static float InternalGetAttenuationRadius(ScriptLight* self);
		static float InternalGetSourceRadius(ScriptLight* self);
		static bool InternalGetUseAutoAttenuation(ScriptLight* self);
		static float InternalGetIntensity(ScriptLight* self);
		static void InternalGetSpotAngle(ScriptLight* self, TDegree<float>* __output);
		static void InternalGetSpotFalloffAngle(ScriptLight* self, TDegree<float>* __output);
		static void InternalGetBounds(ScriptLight* self, __TSphere_float_Interop* __output);
	};
}
