//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Renderer/BsLight.h"
#include "Math/BsSphere.h"
#include "Image/BsColor.h"
#include "Math/BsDegree.h"

namespace bs { class CLight; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptLight : public TScriptComponent<ScriptLight, CLight>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Light")

		ScriptLight(MonoObject* managedInstance, const GameObjectHandle<CLight>& value);

	private:
		static void InternalSetType(ScriptLight* self, LightType type);
		static LightType InternalGetType(ScriptLight* self);
		static void InternalSetColor(ScriptLight* self, Color* color);
		static void InternalGetColor(ScriptLight* self, Color* __output);
		static void InternalSetIntensity(ScriptLight* self, float intensity);
		static float InternalGetIntensity(ScriptLight* self);
		static void InternalSetUseAutoAttenuation(ScriptLight* self, bool enabled);
		static bool InternalGetUseAutoAttenuation(ScriptLight* self);
		static void InternalSetAttenuationRadius(ScriptLight* self, float radius);
		static float InternalGetAttenuationRadius(ScriptLight* self);
		static void InternalSetSourceRadius(ScriptLight* self, float radius);
		static float InternalGetSourceRadius(ScriptLight* self);
		static void InternalSetSpotAngle(ScriptLight* self, TDegree<float>* spotAngle);
		static void InternalGetSpotAngle(ScriptLight* self, TDegree<float>* __output);
		static void InternalSetSpotFalloffAngle(ScriptLight* self, TDegree<float>* spotAngle);
		static void InternalGetSpotFalloffAngle(ScriptLight* self, TDegree<float>* __output);
		static void InternalSetCastsShadow(ScriptLight* self, bool castsShadow);
		static bool InternalGetCastsShadow(ScriptLight* self);
		static void InternalSetShadowBias(ScriptLight* self, float bias);
		static float InternalGetShadowBias(ScriptLight* self);
		static void InternalGetBounds(ScriptLight* self, Sphere* __output);
	};
}
