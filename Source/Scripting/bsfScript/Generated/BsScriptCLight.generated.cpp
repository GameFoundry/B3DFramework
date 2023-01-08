//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptCLight.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Components/BsCLight.h"
#include "Wrappers/BsScriptColor.h"

namespace bs
{
	ScriptLight::ScriptLight(MonoObject* managedInstance, const GameObjectHandle<CLight>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptLight::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptLight::InternalSetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptLight::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_SetColor", (void*)&ScriptLight::InternalSetColor);
		metaData.ScriptClass->AddInternalCall("Internal_GetColor", (void*)&ScriptLight::InternalGetColor);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptLight::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptLight::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseAutoAttenuation", (void*)&ScriptLight::InternalSetUseAutoAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseAutoAttenuation", (void*)&ScriptLight::InternalGetUseAutoAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_SetAttenuationRadius", (void*)&ScriptLight::InternalSetAttenuationRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetAttenuationRadius", (void*)&ScriptLight::InternalGetAttenuationRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetSourceRadius", (void*)&ScriptLight::InternalSetSourceRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetSourceRadius", (void*)&ScriptLight::InternalGetSourceRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpotAngle", (void*)&ScriptLight::InternalSetSpotAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpotAngle", (void*)&ScriptLight::InternalGetSpotAngle);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpotFalloffAngle", (void*)&ScriptLight::InternalSetSpotFalloffAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpotFalloffAngle", (void*)&ScriptLight::InternalGetSpotFalloffAngle);
		metaData.ScriptClass->AddInternalCall("Internal_SetCastsShadow", (void*)&ScriptLight::InternalSetCastsShadow);
		metaData.ScriptClass->AddInternalCall("Internal_GetCastsShadow", (void*)&ScriptLight::InternalGetCastsShadow);
		metaData.ScriptClass->AddInternalCall("Internal_SetShadowBias", (void*)&ScriptLight::InternalSetShadowBias);
		metaData.ScriptClass->AddInternalCall("Internal_GetShadowBias", (void*)&ScriptLight::InternalGetShadowBias);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptLight::InternalGetBounds);

	}

	void ScriptLight::InternalSetType(ScriptLight* thisPtr, LightType type)
	{
		thisPtr->GetHandle()->SetType(type);
	}

	LightType ScriptLight::InternalGetType(ScriptLight* thisPtr)
	{
		LightType tmp__output;
		tmp__output = thisPtr->GetHandle()->GetType();

		LightType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetColor(ScriptLight* thisPtr, Color* color)
	{
		thisPtr->GetHandle()->SetColor(*color);
	}

	void ScriptLight::InternalGetColor(ScriptLight* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetHandle()->GetColor();

		*__output = tmp__output;
	}

	void ScriptLight::InternalSetIntensity(ScriptLight* thisPtr, float intensity)
	{
		thisPtr->GetHandle()->SetIntensity(intensity);
	}

	float ScriptLight::InternalGetIntensity(ScriptLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetIntensity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetUseAutoAttenuation(ScriptLight* thisPtr, bool enabled)
	{
		thisPtr->GetHandle()->SetUseAutoAttenuation(enabled);
	}

	bool ScriptLight::InternalGetUseAutoAttenuation(ScriptLight* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUseAutoAttenuation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetAttenuationRadius(ScriptLight* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetAttenuationRadius(radius);
	}

	float ScriptLight::InternalGetAttenuationRadius(ScriptLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAttenuationRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetSourceRadius(ScriptLight* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetSourceRadius(radius);
	}

	float ScriptLight::InternalGetSourceRadius(ScriptLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSourceRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetSpotAngle(ScriptLight* thisPtr, Degree* spotAngle)
	{
		thisPtr->GetHandle()->SetSpotAngle(*spotAngle);
	}

	void ScriptLight::InternalGetSpotAngle(ScriptLight* thisPtr, Degree* __output)
	{
		Degree tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpotAngle();

		*__output = tmp__output;
	}

	void ScriptLight::InternalSetSpotFalloffAngle(ScriptLight* thisPtr, Degree* spotAngle)
	{
		thisPtr->GetHandle()->SetSpotFalloffAngle(*spotAngle);
	}

	void ScriptLight::InternalGetSpotFalloffAngle(ScriptLight* thisPtr, Degree* __output)
	{
		Degree tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpotFalloffAngle();

		*__output = tmp__output;
	}

	void ScriptLight::InternalSetCastsShadow(ScriptLight* thisPtr, bool castsShadow)
	{
		thisPtr->GetHandle()->SetCastsShadow(castsShadow);
	}

	bool ScriptLight::InternalGetCastsShadow(ScriptLight* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCastsShadow();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetShadowBias(ScriptLight* thisPtr, float bias)
	{
		thisPtr->GetHandle()->SetShadowBias(bias);
	}

	float ScriptLight::InternalGetShadowBias(ScriptLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetShadowBias();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalGetBounds(ScriptLight* thisPtr, Sphere* __output)
	{
		Sphere tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBounds();

		*__output = tmp__output;
	}
}
