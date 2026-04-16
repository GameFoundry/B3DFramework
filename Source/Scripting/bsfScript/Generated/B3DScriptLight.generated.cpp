//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptLight.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Components/B3DLight.h"
#include "B3DScriptColor.generated.h"
#include "B3DScriptTSphere.generated.h"

namespace b3d
{
	ScriptLight::ScriptLight(const TGameObjectHandle<Light>& nativeObject)
		:TScriptGameObjectWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptLight::~ScriptLight()
	{
		UnregisterEvents();
	}

	void ScriptLight::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptLight::InternalSetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetCastsShadow", (void*)&ScriptLight::InternalSetCastsShadow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetShadowBias", (void*)&ScriptLight::InternalSetShadowBias);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetColor", (void*)&ScriptLight::InternalSetColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAttenuationRadius", (void*)&ScriptLight::InternalSetAttenuationRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSourceRadius", (void*)&ScriptLight::InternalSetSourceRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetUseAutoAttenuation", (void*)&ScriptLight::InternalSetUseAutoAttenuation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIntensity", (void*)&ScriptLight::InternalSetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpotAngle", (void*)&ScriptLight::InternalSetSpotAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSpotFalloffAngle", (void*)&ScriptLight::InternalSetSpotFalloffAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptLight::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetCastsShadow", (void*)&ScriptLight::InternalGetCastsShadow);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetShadowBias", (void*)&ScriptLight::InternalGetShadowBias);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetColor", (void*)&ScriptLight::InternalGetColor);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAttenuationRadius", (void*)&ScriptLight::InternalGetAttenuationRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSourceRadius", (void*)&ScriptLight::InternalGetSourceRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetUseAutoAttenuation", (void*)&ScriptLight::InternalGetUseAutoAttenuation);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIntensity", (void*)&ScriptLight::InternalGetIntensity);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpotAngle", (void*)&ScriptLight::InternalGetSpotAngle);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSpotFalloffAngle", (void*)&ScriptLight::InternalGetSpotFalloffAngle);
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
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetType(type);
	}

	void ScriptLight::InternalSetCastsShadow(ScriptLight* self, bool castsShadow)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetCastsShadow(castsShadow);
	}

	void ScriptLight::InternalSetShadowBias(ScriptLight* self, float bias)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetShadowBias(bias);
	}

	void ScriptLight::InternalSetColor(ScriptLight* self, Color* color)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetColor(*color);
	}

	void ScriptLight::InternalSetAttenuationRadius(ScriptLight* self, float radius)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetAttenuationRadius(radius);
	}

	void ScriptLight::InternalSetSourceRadius(ScriptLight* self, float radius)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetSourceRadius(radius);
	}

	void ScriptLight::InternalSetUseAutoAttenuation(ScriptLight* self, bool enabled)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetUseAutoAttenuation(enabled);
	}

	void ScriptLight::InternalSetIntensity(ScriptLight* self, float intensity)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetIntensity(intensity);
	}

	void ScriptLight::InternalSetSpotAngle(ScriptLight* self, TDegree<float>* spotAngle)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetSpotAngle(*spotAngle);
	}

	void ScriptLight::InternalSetSpotFalloffAngle(ScriptLight* self, TDegree<float>* spotFallofAngle)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<Light*>(self->GetNativeObject())->SetSpotFalloffAngle(*spotFallofAngle);
	}

	LightType ScriptLight::InternalGetType(ScriptLight* self)
	{
		LightType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetType();

		LightType __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptLight::InternalGetCastsShadow(ScriptLight* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetCastsShadow();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptLight::InternalGetShadowBias(ScriptLight* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetShadowBias();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalGetColor(ScriptLight* self, Color* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		Color tmp__output;
		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetColor();

		*__output = tmp__output;
	}

	float ScriptLight::InternalGetAttenuationRadius(ScriptLight* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetAttenuationRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptLight::InternalGetSourceRadius(ScriptLight* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetSourceRadius();

		float __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptLight::InternalGetUseAutoAttenuation(ScriptLight* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetUseAutoAttenuation();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	float ScriptLight::InternalGetIntensity(ScriptLight* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetIntensity();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptLight::InternalGetSpotAngle(ScriptLight* self, TDegree<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TDegree<float> tmp__output;
		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetSpotAngle();

		*__output = tmp__output;
	}

	void ScriptLight::InternalGetSpotFalloffAngle(ScriptLight* self, TDegree<float>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TDegree<float> tmp__output;
		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetSpotFalloffAngle();

		*__output = tmp__output;
	}

	void ScriptLight::InternalGetBounds(ScriptLight* self, __TSphere_float_Interop* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TSphere<float> tmp__output;
		tmp__output = static_cast<Light*>(self->GetNativeObject())->GetBounds();

		__TSphere_float_Interop interop__output;
		interop__output = ScriptSphere::ToInterop(tmp__output);
		MonoUtil::ValueCopy(__output, &interop__output, ScriptSphere::GetMetaData()->ScriptClass->GetInternalClass());
	}
}
