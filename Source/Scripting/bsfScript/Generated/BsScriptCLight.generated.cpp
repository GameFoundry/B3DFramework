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
	ScriptCLight::ScriptCLight(MonoObject* managedInstance, const GameObjectHandle<CLight>& value)
		:TScriptComponent(managedInstance, value)
	{
	}

	void ScriptCLight::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptCLight::InternalSetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptCLight::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_SetColor", (void*)&ScriptCLight::InternalSetColor);
		metaData.ScriptClass->AddInternalCall("Internal_GetColor", (void*)&ScriptCLight::InternalGetColor);
		metaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptCLight::InternalSetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptCLight::InternalGetIntensity);
		metaData.ScriptClass->AddInternalCall("Internal_SetUseAutoAttenuation", (void*)&ScriptCLight::InternalSetUseAutoAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_GetUseAutoAttenuation", (void*)&ScriptCLight::InternalGetUseAutoAttenuation);
		metaData.ScriptClass->AddInternalCall("Internal_SetAttenuationRadius", (void*)&ScriptCLight::InternalSetAttenuationRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetAttenuationRadius", (void*)&ScriptCLight::InternalGetAttenuationRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetSourceRadius", (void*)&ScriptCLight::InternalSetSourceRadius);
		metaData.ScriptClass->AddInternalCall("Internal_GetSourceRadius", (void*)&ScriptCLight::InternalGetSourceRadius);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpotAngle", (void*)&ScriptCLight::InternalSetSpotAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpotAngle", (void*)&ScriptCLight::InternalGetSpotAngle);
		metaData.ScriptClass->AddInternalCall("Internal_SetSpotFalloffAngle", (void*)&ScriptCLight::InternalSetSpotFalloffAngle);
		metaData.ScriptClass->AddInternalCall("Internal_GetSpotFalloffAngle", (void*)&ScriptCLight::InternalGetSpotFalloffAngle);
		metaData.ScriptClass->AddInternalCall("Internal_SetCastsShadow", (void*)&ScriptCLight::InternalSetCastsShadow);
		metaData.ScriptClass->AddInternalCall("Internal_GetCastsShadow", (void*)&ScriptCLight::InternalGetCastsShadow);
		metaData.ScriptClass->AddInternalCall("Internal_SetShadowBias", (void*)&ScriptCLight::InternalSetShadowBias);
		metaData.ScriptClass->AddInternalCall("Internal_GetShadowBias", (void*)&ScriptCLight::InternalGetShadowBias);
		metaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptCLight::InternalGetBounds);

	}

	void ScriptCLight::InternalSetType(ScriptCLight* thisPtr, LightType type)
	{
		thisPtr->GetHandle()->SetType(type);
	}

	LightType ScriptCLight::InternalGetType(ScriptCLight* thisPtr)
	{
		LightType tmp__output;
		tmp__output = thisPtr->GetHandle()->GetType();

		LightType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalSetColor(ScriptCLight* thisPtr, Color* color)
	{
		thisPtr->GetHandle()->SetColor(*color);
	}

	void ScriptCLight::InternalGetColor(ScriptCLight* thisPtr, Color* __output)
	{
		Color tmp__output;
		tmp__output = thisPtr->GetHandle()->GetColor();

		*__output = tmp__output;
	}

	void ScriptCLight::InternalSetIntensity(ScriptCLight* thisPtr, float intensity)
	{
		thisPtr->GetHandle()->SetIntensity(intensity);
	}

	float ScriptCLight::InternalGetIntensity(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetIntensity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalSetUseAutoAttenuation(ScriptCLight* thisPtr, bool enabled)
	{
		thisPtr->GetHandle()->SetUseAutoAttenuation(enabled);
	}

	bool ScriptCLight::InternalGetUseAutoAttenuation(ScriptCLight* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetUseAutoAttenuation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalSetAttenuationRadius(ScriptCLight* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetAttenuationRadius(radius);
	}

	float ScriptCLight::InternalGetAttenuationRadius(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetAttenuationRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalSetSourceRadius(ScriptCLight* thisPtr, float radius)
	{
		thisPtr->GetHandle()->SetSourceRadius(radius);
	}

	float ScriptCLight::InternalGetSourceRadius(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSourceRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalSetSpotAngle(ScriptCLight* thisPtr, Degree* spotAngle)
	{
		thisPtr->GetHandle()->SetSpotAngle(*spotAngle);
	}

	void ScriptCLight::InternalGetSpotAngle(ScriptCLight* thisPtr, Degree* __output)
	{
		Degree tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpotAngle();

		*__output = tmp__output;
	}

	void ScriptCLight::InternalSetSpotFalloffAngle(ScriptCLight* thisPtr, Degree* spotAngle)
	{
		thisPtr->GetHandle()->SetSpotFalloffAngle(*spotAngle);
	}

	void ScriptCLight::InternalGetSpotFalloffAngle(ScriptCLight* thisPtr, Degree* __output)
	{
		Degree tmp__output;
		tmp__output = thisPtr->GetHandle()->GetSpotFalloffAngle();

		*__output = tmp__output;
	}

	void ScriptCLight::InternalSetCastsShadow(ScriptCLight* thisPtr, bool castsShadow)
	{
		thisPtr->GetHandle()->SetCastsShadow(castsShadow);
	}

	bool ScriptCLight::InternalGetCastsShadow(ScriptCLight* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetHandle()->GetCastsShadow();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalSetShadowBias(ScriptCLight* thisPtr, float bias)
	{
		thisPtr->GetHandle()->SetShadowBias(bias);
	}

	float ScriptCLight::InternalGetShadowBias(ScriptCLight* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetHandle()->GetShadowBias();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptCLight::InternalGetBounds(ScriptCLight* thisPtr, Sphere* __output)
	{
		Sphere tmp__output;
		tmp__output = thisPtr->GetHandle()->GetBounds();

		*__output = tmp__output;
	}
}
