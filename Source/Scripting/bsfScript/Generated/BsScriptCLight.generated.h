//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	class BS_SCR_BE_EXPORT ScriptCLight : public TScriptComponent<ScriptCLight, CLight>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Light")

		ScriptCLight(MonoObject* managedInstance, const GameObjectHandle<CLight>& value);

	private:
		static void Internal_setType(ScriptCLight* thisPtr, LightType type);
		static LightType Internal_getType(ScriptCLight* thisPtr);
		static void Internal_setColor(ScriptCLight* thisPtr, Color* color);
		static void Internal_getColor(ScriptCLight* thisPtr, Color* __output);
		static void Internal_setIntensity(ScriptCLight* thisPtr, float intensity);
		static float Internal_getIntensity(ScriptCLight* thisPtr);
		static void Internal_setUseAutoAttenuation(ScriptCLight* thisPtr, bool enabled);
		static bool Internal_getUseAutoAttenuation(ScriptCLight* thisPtr);
		static void Internal_setAttenuationRadius(ScriptCLight* thisPtr, float radius);
		static float Internal_getAttenuationRadius(ScriptCLight* thisPtr);
		static void Internal_setSourceRadius(ScriptCLight* thisPtr, float radius);
		static float Internal_getSourceRadius(ScriptCLight* thisPtr);
		static void Internal_setSpotAngle(ScriptCLight* thisPtr, Degree* spotAngle);
		static void Internal_getSpotAngle(ScriptCLight* thisPtr, Degree* __output);
		static void Internal_setSpotFalloffAngle(ScriptCLight* thisPtr, Degree* spotAngle);
		static void Internal_getSpotFalloffAngle(ScriptCLight* thisPtr, Degree* __output);
		static void Internal_setCastsShadow(ScriptCLight* thisPtr, bool castsShadow);
		static bool Internal_getCastsShadow(ScriptCLight* thisPtr);
		static void Internal_setShadowBias(ScriptCLight* thisPtr, float bias);
		static float Internal_getShadowBias(ScriptCLight* thisPtr);
		static void Internal_getBounds(ScriptCLight* thisPtr, Sphere* __output);
	};
}
