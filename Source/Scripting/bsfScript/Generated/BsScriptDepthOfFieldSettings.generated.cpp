//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptDepthOfFieldSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsScriptResourceManager.h"
#include "Wrappers/BsScriptRRefBase.h"
#include "../../../Foundation/bsfCore/Image/BsTexture.h"
#include "Wrappers/BsScriptVector.h"

namespace bs
{
	ScriptDepthOfFieldSettings::ScriptDepthOfFieldSettings(MonoObject* managedInstance, const SPtr<DepthOfFieldSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptDepthOfFieldSettings::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_DepthOfFieldSettings", (void*)&ScriptDepthOfFieldSettings::InternalDepthOfFieldSettings);
		metaData.ScriptClass->AddInternalCall("Internal_GetBokehShape", (void*)&ScriptDepthOfFieldSettings::InternalGetBokehShape);
		metaData.ScriptClass->AddInternalCall("Internal_SetBokehShape", (void*)&ScriptDepthOfFieldSettings::InternalSetBokehShape);
		metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptDepthOfFieldSettings::InternalGetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptDepthOfFieldSettings::InternalSetEnabled);
		metaData.ScriptClass->AddInternalCall("Internal_GetType", (void*)&ScriptDepthOfFieldSettings::InternalGetType);
		metaData.ScriptClass->AddInternalCall("Internal_SetType", (void*)&ScriptDepthOfFieldSettings::InternalSetType);
		metaData.ScriptClass->AddInternalCall("Internal_GetFocalDistance", (void*)&ScriptDepthOfFieldSettings::InternalGetFocalDistance);
		metaData.ScriptClass->AddInternalCall("Internal_SetFocalDistance", (void*)&ScriptDepthOfFieldSettings::InternalSetFocalDistance);
		metaData.ScriptClass->AddInternalCall("Internal_GetFocalRange", (void*)&ScriptDepthOfFieldSettings::InternalGetFocalRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetFocalRange", (void*)&ScriptDepthOfFieldSettings::InternalSetFocalRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetNearTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalGetNearTransitionRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetNearTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalSetNearTransitionRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetFarTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalGetFarTransitionRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetFarTransitionRange", (void*)&ScriptDepthOfFieldSettings::InternalSetFarTransitionRange);
		metaData.ScriptClass->AddInternalCall("Internal_GetNearBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalGetNearBlurAmount);
		metaData.ScriptClass->AddInternalCall("Internal_SetNearBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalSetNearBlurAmount);
		metaData.ScriptClass->AddInternalCall("Internal_GetFarBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalGetFarBlurAmount);
		metaData.ScriptClass->AddInternalCall("Internal_SetFarBlurAmount", (void*)&ScriptDepthOfFieldSettings::InternalSetFarBlurAmount);
		metaData.ScriptClass->AddInternalCall("Internal_GetMaxBokehSize", (void*)&ScriptDepthOfFieldSettings::InternalGetMaxBokehSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetMaxBokehSize", (void*)&ScriptDepthOfFieldSettings::InternalSetMaxBokehSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetAdaptiveColorThreshold", (void*)&ScriptDepthOfFieldSettings::InternalGetAdaptiveColorThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetAdaptiveColorThreshold", (void*)&ScriptDepthOfFieldSettings::InternalSetAdaptiveColorThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetAdaptiveRadiusThreshold", (void*)&ScriptDepthOfFieldSettings::InternalGetAdaptiveRadiusThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_SetAdaptiveRadiusThreshold", (void*)&ScriptDepthOfFieldSettings::InternalSetAdaptiveRadiusThreshold);
		metaData.ScriptClass->AddInternalCall("Internal_GetApertureSize", (void*)&ScriptDepthOfFieldSettings::InternalGetApertureSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetApertureSize", (void*)&ScriptDepthOfFieldSettings::InternalSetApertureSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetFocalLength", (void*)&ScriptDepthOfFieldSettings::InternalGetFocalLength);
		metaData.ScriptClass->AddInternalCall("Internal_SetFocalLength", (void*)&ScriptDepthOfFieldSettings::InternalSetFocalLength);
		metaData.ScriptClass->AddInternalCall("Internal_GetSensorSize", (void*)&ScriptDepthOfFieldSettings::InternalGetSensorSize);
		metaData.ScriptClass->AddInternalCall("Internal_SetSensorSize", (void*)&ScriptDepthOfFieldSettings::InternalSetSensorSize);
		metaData.ScriptClass->AddInternalCall("Internal_GetBokehOcclusion", (void*)&ScriptDepthOfFieldSettings::InternalGetBokehOcclusion);
		metaData.ScriptClass->AddInternalCall("Internal_SetBokehOcclusion", (void*)&ScriptDepthOfFieldSettings::InternalSetBokehOcclusion);
		metaData.ScriptClass->AddInternalCall("Internal_GetOcclusionDepthRange", (void*)&ScriptDepthOfFieldSettings::InternalGetOcclusionDepthRange);
		metaData.ScriptClass->AddInternalCall("Internal_SetOcclusionDepthRange", (void*)&ScriptDepthOfFieldSettings::InternalSetOcclusionDepthRange);

	}

	MonoObject* ScriptDepthOfFieldSettings::Create(const SPtr<DepthOfFieldSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptDepthOfFieldSettings>()) ScriptDepthOfFieldSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptDepthOfFieldSettings::InternalDepthOfFieldSettings(MonoObject* managedInstance)
	{
		SPtr<DepthOfFieldSettings> nativeObject = B3DMakeShared<DepthOfFieldSettings>();
		new (B3DAllocate<ScriptDepthOfFieldSettings>())ScriptDepthOfFieldSettings(managedInstance, nativeObject);
	}

	MonoObject* ScriptDepthOfFieldSettings::InternalGetBokehShape(ScriptDepthOfFieldSettings* self)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = self->GetInternal()->BokehShape;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetBokehShape(ScriptDepthOfFieldSettings* self, MonoObject* value)
	{
		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetHandle());
		self->GetInternal()->BokehShape = tmpvalue;
	}

	bool ScriptDepthOfFieldSettings::InternalGetEnabled(ScriptDepthOfFieldSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetEnabled(ScriptDepthOfFieldSettings* self, bool value)
	{
		self->GetInternal()->Enabled = value;
	}

	DepthOfFieldType ScriptDepthOfFieldSettings::InternalGetType(ScriptDepthOfFieldSettings* self)
	{
		DepthOfFieldType tmp__output;
		tmp__output = self->GetInternal()->Type;

		DepthOfFieldType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetType(ScriptDepthOfFieldSettings* self, DepthOfFieldType value)
	{
		self->GetInternal()->Type = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalDistance(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FocalDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalDistance(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->FocalDistance = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FocalRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalRange(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->FocalRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetNearTransitionRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->NearTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetNearTransitionRange(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->NearTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFarTransitionRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FarTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFarTransitionRange(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->FarTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetNearBlurAmount(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->NearBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetNearBlurAmount(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->NearBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFarBlurAmount(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FarBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFarBlurAmount(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->FarBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetMaxBokehSize(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->MaxBokehSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetMaxBokehSize(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->MaxBokehSize = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->AdaptiveColorThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->AdaptiveColorThreshold = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->AdaptiveRadiusThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->AdaptiveRadiusThreshold = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetApertureSize(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->ApertureSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetApertureSize(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->ApertureSize = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalLength(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->FocalLength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalLength(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->FocalLength = value;
	}

	void ScriptDepthOfFieldSettings::InternalGetSensorSize(ScriptDepthOfFieldSettings* self, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = self->GetInternal()->SensorSize;

		*__output = tmp__output;


	}

	void ScriptDepthOfFieldSettings::InternalSetSensorSize(ScriptDepthOfFieldSettings* self, TVector2<float>* value)
	{
		self->GetInternal()->SensorSize = *value;
	}

	bool ScriptDepthOfFieldSettings::InternalGetBokehOcclusion(ScriptDepthOfFieldSettings* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->BokehOcclusion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetBokehOcclusion(ScriptDepthOfFieldSettings* self, bool value)
	{
		self->GetInternal()->BokehOcclusion = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetOcclusionDepthRange(ScriptDepthOfFieldSettings* self)
	{
		float tmp__output;
		tmp__output = self->GetInternal()->OcclusionDepthRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetOcclusionDepthRange(ScriptDepthOfFieldSettings* self, float value)
	{
		self->GetInternal()->OcclusionDepthRange = value;
	}
}
