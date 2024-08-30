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
	ScriptLight::ScriptLight(const GameObjectHandle<CLight>& nativeObject, MonoObject* scriptObject)
		:TScriptGameObjectWrapper(nativeObject, scriptObject)
	{
		RegisterEvents();
	}

	void ScriptLight::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptLight::InternalSetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptLight::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColor", (void*)&ScriptLight::InternalSetColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColor", (void*)&ScriptLight::InternalGetColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptLight::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptLight::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseAutoAttenuation", (void*)&ScriptLight::InternalSetUseAutoAttenuation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseAutoAttenuation", (void*)&ScriptLight::InternalGetUseAutoAttenuation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAttenuationRadius", (void*)&ScriptLight::InternalSetAttenuationRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAttenuationRadius", (void*)&ScriptLight::InternalGetAttenuationRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSourceRadius", (void*)&ScriptLight::InternalSetSourceRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSourceRadius", (void*)&ScriptLight::InternalGetSourceRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpotAngle", (void*)&ScriptLight::InternalSetSpotAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpotAngle", (void*)&ScriptLight::InternalGetSpotAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpotFalloffAngle", (void*)&ScriptLight::InternalSetSpotFalloffAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpotFalloffAngle", (void*)&ScriptLight::InternalGetSpotFalloffAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCastsShadow", (void*)&ScriptLight::InternalSetCastsShadow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCastsShadow", (void*)&ScriptLight::InternalGetCastsShadow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShadowBias", (void*)&ScriptLight::InternalSetShadowBias);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShadowBias", (void*)&ScriptLight::InternalGetShadowBias);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBounds", (void*)&ScriptLight::InternalGetBounds);

	}

	MonoObject* ScriptLight::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptLight::InternalSetType(ScriptLight* self, LightType type)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetType(type);
	}

	LightType ScriptLight::InternalGetType(ScriptLight* self)
	{
		LightType tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetType();

		LightType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetColor(ScriptLight* self, Color* color)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetColor(*color);
	}

	void ScriptLight::InternalGetColor(ScriptLight* self, Color* __output)
	{
		Color tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetColor();

		*__output = tmp__output;
	}

	void ScriptLight::InternalSetIntensity(ScriptLight* self, float intensity)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetIntensity(intensity);
	}

	float ScriptLight::InternalGetIntensity(ScriptLight* self)
	{
		float tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetIntensity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetUseAutoAttenuation(ScriptLight* self, bool enabled)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetUseAutoAttenuation(enabled);
	}

	bool ScriptLight::InternalGetUseAutoAttenuation(ScriptLight* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetUseAutoAttenuation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetAttenuationRadius(ScriptLight* self, float radius)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetAttenuationRadius(radius);
	}

	float ScriptLight::InternalGetAttenuationRadius(ScriptLight* self)
	{
		float tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetAttenuationRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetSourceRadius(ScriptLight* self, float radius)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetSourceRadius(radius);
	}

	float ScriptLight::InternalGetSourceRadius(ScriptLight* self)
	{
		float tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetSourceRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetSpotAngle(ScriptLight* self, TDegree<float>* spotAngle)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetSpotAngle(*spotAngle);
	}

	void ScriptLight::InternalGetSpotAngle(ScriptLight* self, TDegree<float>* __output)
	{
		TDegree<float> tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetSpotAngle();

		*__output = tmp__output;
	}

	void ScriptLight::InternalSetSpotFalloffAngle(ScriptLight* self, TDegree<float>* spotAngle)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetSpotFalloffAngle(*spotAngle);
	}

	void ScriptLight::InternalGetSpotFalloffAngle(ScriptLight* self, TDegree<float>* __output)
	{
		TDegree<float> tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetSpotFalloffAngle();

		*__output = tmp__output;
	}

	void ScriptLight::InternalSetCastsShadow(ScriptLight* self, bool castsShadow)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetCastsShadow(castsShadow);
	}

	bool ScriptLight::InternalGetCastsShadow(ScriptLight* self)
	{
		bool tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetCastsShadow();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalSetShadowBias(ScriptLight* self, float bias)
	{
		static_cast<CLight*>(self->GetNativeObject())->SetShadowBias(bias);
	}

	float ScriptLight::InternalGetShadowBias(ScriptLight* self)
	{
		float tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetShadowBias();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalGetBounds(ScriptLight* self, Sphere* __output)
	{
		Sphere tmp__output;
		tmp__output = static_cast<CLight*>(self->GetNativeObject())->GetBounds();

		*__output = tmp__output;
	}
}
