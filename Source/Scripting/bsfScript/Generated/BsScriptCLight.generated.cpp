//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCLight.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCLight.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptCLight::ScriptCLight(MonoObject* managedInstance, const GameObjectHandle<CLight>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCLight::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_setType", (void*)&ScriptCLight::Internal_setType);
		metaData.scriptClass->addInternalCall("Internal_getType", (void*)&ScriptCLight::Internal_getType);
		metaData.scriptClass->addInternalCall("Internal_setColor", (void*)&ScriptCLight::Internal_setColor);
		metaData.scriptClass->addInternalCall("Internal_getColor", (void*)&ScriptCLight::Internal_getColor);
		metaData.scriptClass->addInternalCall("Internal_setIntensity", (void*)&ScriptCLight::Internal_setIntensity);
		metaData.scriptClass->addInternalCall("Internal_getIntensity", (void*)&ScriptCLight::Internal_getIntensity);
		metaData.scriptClass->addInternalCall("Internal_setUseAutoAttenuation", (void*)&ScriptCLight::Internal_setUseAutoAttenuation);
		metaData.scriptClass->addInternalCall("Internal_getUseAutoAttenuation", (void*)&ScriptCLight::Internal_getUseAutoAttenuation);
		metaData.scriptClass->addInternalCall("Internal_setAttenuationRadius", (void*)&ScriptCLight::Internal_setAttenuationRadius);
		metaData.scriptClass->addInternalCall("Internal_getAttenuationRadius", (void*)&ScriptCLight::Internal_getAttenuationRadius);
		metaData.scriptClass->addInternalCall("Internal_setSourceRadius", (void*)&ScriptCLight::Internal_setSourceRadius);
		metaData.scriptClass->addInternalCall("Internal_getSourceRadius", (void*)&ScriptCLight::Internal_getSourceRadius);
		metaData.scriptClass->addInternalCall("Internal_setSpotAngle", (void*)&ScriptCLight::Internal_setSpotAngle);
		metaData.scriptClass->addInternalCall("Internal_getSpotAngle", (void*)&ScriptCLight::Internal_getSpotAngle);
		metaData.scriptClass->addInternalCall("Internal_setSpotFalloffAngle", (void*)&ScriptCLight::Internal_setSpotFalloffAngle);
		metaData.scriptClass->addInternalCall("Internal_getSpotFalloffAngle", (void*)&ScriptCLight::Internal_getSpotFalloffAngle);
		metaData.scriptClass->addInternalCall("Internal_setCastsShadow", (void*)&ScriptCLight::Internal_setCastsShadow);
		metaData.scriptClass->addInternalCall("Internal_getCastsShadow", (void*)&ScriptCLight::Internal_getCastsShadow);
		metaData.scriptClass->addInternalCall("Internal_setShadowBias", (void*)&ScriptCLight::Internal_setShadowBias);
		metaData.scriptClass->addInternalCall("Internal_getShadowBias", (void*)&ScriptCLight::Internal_getShadowBias);
		metaData.scriptClass->addInternalCall("Internal_getBounds", (void*)&ScriptCLight::Internal_getBounds);

	}

	void ScriptCLight::Internal_setType(ScriptCLight* thisPtr, LightType type)
	{
		thisPtr->getHandle()->setType(type);
	}

	LightType ScriptCLight::Internal_getType(ScriptCLight* thisPtr)
	{
		LightType tmp__output;
		tmp__output = thisPtr->getHandle()->getType();

		LightType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_setColor(ScriptCLight* thisPtr, Color* color)
	{
		thisPtr->getHandle()->setColor(*color);
	}

	void ScriptCLight::Internal_getColor(ScriptCLight* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->getHandle()->getColor();

		*__output = tmp__output;
	}

	void ScriptCLight::Internal_setIntensity(ScriptCLight* thisPtr, float intensity)
	{
		thisPtr->getHandle()->setIntensity(intensity);
	}

	float ScriptCLight::Internal_getIntensity(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getIntensity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_setUseAutoAttenuation(ScriptCLight* thisPtr, bool enabled)
	{
		thisPtr->getHandle()->setUseAutoAttenuation(enabled);
	}

	bool ScriptCLight::Internal_getUseAutoAttenuation(ScriptCLight* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getUseAutoAttenuation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_setAttenuationRadius(ScriptCLight* thisPtr, float radius)
	{
		thisPtr->getHandle()->setAttenuationRadius(radius);
	}

	float ScriptCLight::Internal_getAttenuationRadius(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getAttenuationRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_setSourceRadius(ScriptCLight* thisPtr, float radius)
	{
		thisPtr->getHandle()->setSourceRadius(radius);
	}

	float ScriptCLight::Internal_getSourceRadius(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getSourceRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_setSpotAngle(ScriptCLight* thisPtr, Degree* spotAngle)
	{
		thisPtr->getHandle()->setSpotAngle(*spotAngle);
	}

	void ScriptCLight::Internal_getSpotAngle(ScriptCLight* thisPtr, Degree* __output)
	{
		Degree tmp__output;
		tmp__output = thisPtr->getHandle()->getSpotAngle();

		*__output = tmp__output;
	}

	void ScriptCLight::Internal_setSpotFalloffAngle(ScriptCLight* thisPtr, Degree* spotAngle)
	{
		thisPtr->getHandle()->setSpotFalloffAngle(*spotAngle);
	}

	void ScriptCLight::Internal_getSpotFalloffAngle(ScriptCLight* thisPtr, Degree* __output)
	{
		Degree tmp__output;
		tmp__output = thisPtr->getHandle()->getSpotFalloffAngle();

		*__output = tmp__output;
	}

	void ScriptCLight::Internal_setCastsShadow(ScriptCLight* thisPtr, bool castsShadow)
	{
		thisPtr->getHandle()->setCastsShadow(castsShadow);
	}

	bool ScriptCLight::Internal_getCastsShadow(ScriptCLight* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getHandle()->getCastsShadow();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_setShadowBias(ScriptCLight* thisPtr, float bias)
	{
		thisPtr->getHandle()->setShadowBias(bias);
	}

	float ScriptCLight::Internal_getShadowBias(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getHandle()->getShadowBias();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::Internal_getBounds(ScriptCLight* thisPtr, Sphere* __output)
	{
		Sphere tmp__output;
		tmp__output = thisPtr->getHandle()->getBounds();

		*__output = tmp__output;
	}
}
