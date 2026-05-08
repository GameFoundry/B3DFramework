//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptDepthOfFieldSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "B3DScriptResourceManager.h"
#include "Wrappers/B3DScriptRRefBase.h"
#include "../../../Engine/Core/Image/B3DTexture.h"
#include "B3DScriptTVector2.generated.h"

namespace b3d
{
	ScriptDepthOfFieldSettings::ScriptDepthOfFieldSettings(const TShared<DepthOfFieldSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptDepthOfFieldSettings::~ScriptDepthOfFieldSettings()
	{
		UnregisterEvents();
	}

	void ScriptDepthOfFieldSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DepthOfFieldSettings", (void*)&ScriptDepthOfFieldSettings::InternalDepthOfFieldSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBokehShape", (void*)&ScriptDepthOfFieldSettings::InternalGetBokehShape);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBokehShape", (void*)&ScriptDepthOfFieldSettings::InternalSetBokehShape);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptDepthOfFieldSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptDepthOfFieldSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptDepthOfFieldSettings::InternalGetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptDepthOfFieldSettings::InternalSetType);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFocalDistance", (void*)&ScriptDepthOfFieldSettings::InternalGetFocalDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFocalDistance", (void*)&ScriptDepthOfFieldSettings::InternalSetFocalDistance);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFocalRange", (void*)&ScriptDepthOfFieldSettings::InternalGetFocalRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFocalRange", (void*)&ScriptDepthOfFieldSettings::InternalSetFocalRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNearTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalGetNearTransitionRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNearTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalSetNearTransitionRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFarTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalGetFarTransitionRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFarTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalSetFarTransitionRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetNearBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalGetNearBlurAmount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetNearBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalSetNearBlurAmount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFarBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalGetFarBlurAmount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFarBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalSetFarBlurAmount);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaxBokehSize", (void*)&ScriptDepthOfFieldSettings::InternalGetMaxBokehSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaxBokehSize", (void*)&ScriptDepthOfFieldSettings::InternalSetMaxBokehSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAdaptiveColorThreshold", (void*)&ScriptDepthOfFieldSettings::InternalGetAdaptiveColorThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAdaptiveColorThreshold", (void*)&ScriptDepthOfFieldSettings::InternalSetAdaptiveColorThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetAdaptiveRadiusThreshold", (void*)&ScriptDepthOfFieldSettings::InternalGetAdaptiveRadiusThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetAdaptiveRadiusThreshold", (void*)&ScriptDepthOfFieldSettings::InternalSetAdaptiveRadiusThreshold);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetApertureSize", (void*)&ScriptDepthOfFieldSettings::InternalGetApertureSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetApertureSize", (void*)&ScriptDepthOfFieldSettings::InternalSetApertureSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFocalLength", (void*)&ScriptDepthOfFieldSettings::InternalGetFocalLength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFocalLength", (void*)&ScriptDepthOfFieldSettings::InternalSetFocalLength);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSensorSize", (void*)&ScriptDepthOfFieldSettings::InternalGetSensorSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSensorSize", (void*)&ScriptDepthOfFieldSettings::InternalSetSensorSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetBokehOcclusion", (void*)&ScriptDepthOfFieldSettings::InternalGetBokehOcclusion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetBokehOcclusion", (void*)&ScriptDepthOfFieldSettings::InternalSetBokehOcclusion);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetOcclusionDepthRange", (void*)&ScriptDepthOfFieldSettings::InternalGetOcclusionDepthRange);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetOcclusionDepthRange", (void*)&ScriptDepthOfFieldSettings::InternalSetOcclusionDepthRange);

	}

	MonoObject* ScriptDepthOfFieldSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptDepthOfFieldSettings::InternalDepthOfFieldSettings(MonoObject* scriptObject)
	{
		TShared<DepthOfFieldSettings> nativeObject = B3DMakeShared<DepthOfFieldSettings>();
		ScriptObjectWrapper::Create<ScriptDepthOfFieldSettings>(nativeObject, scriptObject);
	}

	MonoObject* ScriptDepthOfFieldSettings::InternalGetBokehShape(ScriptDepthOfFieldSettings* self)
	{
		TResourceHandle<Texture> tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->BokehShape;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetScriptObject();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetBokehShape(ScriptDepthOfFieldSettings* self, MonoObject* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::GetScriptObjectWrapper(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetNativeObject());
		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->BokehShape = tmpvalue;
	}

	bool ScriptDepthOfFieldSettings::InternalGetEnabled(ScriptDepthOfFieldSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetEnabled(ScriptDepthOfFieldSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->Enabled = value;
	}

	DepthOfFieldType ScriptDepthOfFieldSettings::InternalGetType(ScriptDepthOfFieldSettings* self)
	{
		DepthOfFieldType tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->Type;

		DepthOfFieldType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetType(ScriptDepthOfFieldSettings* self, DepthOfFieldType value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->Type = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalDistance(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FocalDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalDistance(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FocalDistance = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FocalRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalRange(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FocalRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetNearTransitionRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->NearTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetNearTransitionRange(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->NearTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFarTransitionRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FarTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFarTransitionRange(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FarTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetNearBlurAmount(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->NearBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetNearBlurAmount(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->NearBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFarBlurAmount(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FarBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFarBlurAmount(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FarBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetMaxBokehSize(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->MaxBokehSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetMaxBokehSize(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->MaxBokehSize = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->AdaptiveColorThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->AdaptiveColorThreshold = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->AdaptiveRadiusThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->AdaptiveRadiusThreshold = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetApertureSize(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->ApertureSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetApertureSize(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->ApertureSize = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalLength(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FocalLength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalLength(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->FocalLength = value;
	}

	void ScriptDepthOfFieldSettings::InternalGetSensorSize(ScriptDepthOfFieldSettings* self, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->SensorSize;

		*__output = tmp__output;


	}

	void ScriptDepthOfFieldSettings::InternalSetSensorSize(ScriptDepthOfFieldSettings* self, TVector2<float>* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->SensorSize = *value;
	}

	bool ScriptDepthOfFieldSettings::InternalGetBokehOcclusion(ScriptDepthOfFieldSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->BokehOcclusion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetBokehOcclusion(ScriptDepthOfFieldSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->BokehOcclusion = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetOcclusionDepthRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->OcclusionDepthRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetOcclusionDepthRange(ScriptDepthOfFieldSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<DepthOfFieldSettings*>(self->GetNativeObject())->OcclusionDepthRange = value;
	}
}
