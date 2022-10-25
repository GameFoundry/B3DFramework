//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Renderer/BsLight.h"
#include "Math/BsSphere.h"
#include "Image/BsColor.h"
#include "Math/BsDegree.h"

namespace bs
{
	class CLight;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCLight : public TScriptComponent<ScriptCLight, CLight>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Light")

		ScriptCLight(MonoObject* managedInstance, const GameObjectHandle<CLight>& value);

	private:
		static void InternalSetType(ScriptCLight* thisPtr, LightType type);
		static LightType InternalGetType(ScriptCLight* thisPtr);
		static void InternalSetColor(ScriptCLight* thisPtr, Color* color);
		static void InternalGetColor(ScriptCLight* thisPtr, Color* __output);
		static void InternalSetIntensity(ScriptCLight* thisPtr, float intensity);
		static float InternalGetIntensity(ScriptCLight* thisPtr);
		static void InternalSetUseAutoAttenuation(ScriptCLight* thisPtr, bool enabled);
		static bool InternalGetUseAutoAttenuation(ScriptCLight* thisPtr);
		static void InternalSetAttenuationRadius(ScriptCLight* thisPtr, float radius);
		static float InternalGetAttenuationRadius(ScriptCLight* thisPtr);
		static void InternalSetSourceRadius(ScriptCLight* thisPtr, float radius);
		static float InternalGetSourceRadius(ScriptCLight* thisPtr);
		static void InternalSetSpotAngle(ScriptCLight* thisPtr, Degree* spotAngle);
		static void InternalGetSpotAngle(ScriptCLight* thisPtr, Degree* __output);
		static void InternalSetSpotFalloffAngle(ScriptCLight* thisPtr, Degree* spotAngle);
		static void InternalGetSpotFalloffAngle(ScriptCLight* thisPtr, Degree* __output);
		static void InternalSetCastsShadow(ScriptCLight* thisPtr, bool castsShadow);
		static bool InternalGetCastsShadow(ScriptCLight* thisPtr);
		static void InternalSetShadowBias(ScriptCLight* thisPtr, float bias);
		static float InternalGetShadowBias(ScriptCLight* thisPtr);
		static void InternalGetBounds(ScriptCLight* thisPtr, Sphere* __output);
	};
} // namespace bs
