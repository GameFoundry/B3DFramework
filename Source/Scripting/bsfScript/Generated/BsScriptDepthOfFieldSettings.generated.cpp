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
		SPtr<DepthOfFieldSettings> instance = B3DMakeShared<DepthOfFieldSettings>();
		new (B3DAllocate<ScriptDepthOfFieldSettings>())ScriptDepthOfFieldSettings(managedInstance, instance);
	}

	MonoObject* ScriptDepthOfFieldSettings::InternalGetBokehShape(ScriptDepthOfFieldSettings* thisPtr)
	{
		TResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->GetInternal()->BokehShape;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::Instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetBokehShape(ScriptDepthOfFieldSettings* thisPtr, MonoObject* value)
	{
		TResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptObjectWrappervalue;
		scriptObjectWrappervalue = ScriptRRefBase::ToNative(value);
		if(scriptObjectWrappervalue != nullptr)
			tmpvalue = B3DStaticResourceCast<Texture>(scriptObjectWrappervalue->GetHandle());
		thisPtr->GetInternal()->BokehShape = tmpvalue;
	}

	bool ScriptDepthOfFieldSettings::InternalGetEnabled(ScriptDepthOfFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetEnabled(ScriptDepthOfFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->Enabled = value;
	}

	DepthOfFieldType ScriptDepthOfFieldSettings::InternalGetType(ScriptDepthOfFieldSettings* thisPtr)
	{
		DepthOfFieldType tmp__output;
		tmp__output = thisPtr->GetInternal()->Type;

		DepthOfFieldType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetType(ScriptDepthOfFieldSettings* thisPtr, DepthOfFieldType value)
	{
		thisPtr->GetInternal()->Type = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalDistance(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FocalDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalDistance(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FocalDistance = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FocalRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FocalRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetNearTransitionRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->NearTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetNearTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->NearTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFarTransitionRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FarTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFarTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FarTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetNearBlurAmount(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->NearBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetNearBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->NearBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFarBlurAmount(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FarBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFarBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FarBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetMaxBokehSize(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->MaxBokehSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetMaxBokehSize(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->MaxBokehSize = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->AdaptiveColorThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetAdaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->AdaptiveColorThreshold = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->AdaptiveRadiusThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetAdaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->AdaptiveRadiusThreshold = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetApertureSize(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->ApertureSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetApertureSize(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->ApertureSize = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetFocalLength(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->FocalLength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetFocalLength(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->FocalLength = value;
	}

	void ScriptDepthOfFieldSettings::InternalGetSensorSize(ScriptDepthOfFieldSettings* thisPtr, TVector2<float>* __output)
	{
		TVector2<float> tmp__output;
		tmp__output = thisPtr->GetInternal()->SensorSize;

		*__output = tmp__output;


	}

	void ScriptDepthOfFieldSettings::InternalSetSensorSize(ScriptDepthOfFieldSettings* thisPtr, TVector2<float>* value)
	{
		thisPtr->GetInternal()->SensorSize = *value;
	}

	bool ScriptDepthOfFieldSettings::InternalGetBokehOcclusion(ScriptDepthOfFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->BokehOcclusion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetBokehOcclusion(ScriptDepthOfFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->BokehOcclusion = value;
	}

	float ScriptDepthOfFieldSettings::InternalGetOcclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->OcclusionDepthRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::InternalSetOcclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->OcclusionDepthRange = value;
	}
}
