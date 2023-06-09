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
		static void InternalSetType(ScriptLight* thisPtr, LightType type);
		static LightType InternalGetType(ScriptLight* thisPtr);
		static void InternalSetColor(ScriptLight* thisPtr, Color* color);
		static void InternalGetColor(ScriptLight* thisPtr, Color* __output);
		static void InternalSetIntensity(ScriptLight* thisPtr, float intensity);
		static float InternalGetIntensity(ScriptLight* thisPtr);
		static void InternalSetUseAutoAttenuation(ScriptLight* thisPtr, bool enabled);
		static bool InternalGetUseAutoAttenuation(ScriptLight* thisPtr);
		static void InternalSetAttenuationRadius(ScriptLight* thisPtr, float radius);
		static float InternalGetAttenuationRadius(ScriptLight* thisPtr);
		static void InternalSetSourceRadius(ScriptLight* thisPtr, float radius);
		static float InternalGetSourceRadius(ScriptLight* thisPtr);
		static void InternalSetSpotAngle(ScriptLight* thisPtr, TDegree<float>* spotAngle);
		static void InternalGetSpotAngle(ScriptLight* thisPtr, TDegree<float>* __output);
		static void InternalSetSpotFalloffAngle(ScriptLight* thisPtr, TDegree<float>* spotAngle);
		static void InternalGetSpotFalloffAngle(ScriptLight* thisPtr, TDegree<float>* __output);
		static void InternalSetCastsShadow(ScriptLight* thisPtr, bool castsShadow);
		static bool InternalGetCastsShadow(ScriptLight* thisPtr);
		static void InternalSetShadowBias(ScriptLight* thisPtr, float bias);
		static float InternalGetShadowBias(ScriptLight* thisPtr);
		static void InternalGetBounds(ScriptLight* thisPtr, Sphere* __output);
	};
}
