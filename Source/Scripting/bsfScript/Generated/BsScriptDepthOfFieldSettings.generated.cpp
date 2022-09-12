//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
		metaData.scriptClass->AddInternalCall("Internal_DepthOfFieldSettings", (void*)&ScriptDepthOfFieldSettings::Internal_DepthOfFieldSettings);
		metaData.scriptClass->AddInternalCall("Internal_getbokehShape", (void*)&ScriptDepthOfFieldSettings::Internal_getbokehShape);
		metaData.scriptClass->AddInternalCall("Internal_setbokehShape", (void*)&ScriptDepthOfFieldSettings::Internal_setbokehShape);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptDepthOfFieldSettings::Internal_getenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptDepthOfFieldSettings::Internal_setenabled);
		metaData.scriptClass->AddInternalCall("Internal_gettype", (void*)&ScriptDepthOfFieldSettings::Internal_gettype);
		metaData.scriptClass->AddInternalCall("Internal_settype", (void*)&ScriptDepthOfFieldSettings::Internal_settype);
		metaData.scriptClass->AddInternalCall("Internal_getfocalDistance", (void*)&ScriptDepthOfFieldSettings::Internal_getfocalDistance);
		metaData.scriptClass->AddInternalCall("Internal_setfocalDistance", (void*)&ScriptDepthOfFieldSettings::Internal_setfocalDistance);
		metaData.scriptClass->AddInternalCall("Internal_getfocalRange", (void*)&ScriptDepthOfFieldSettings::Internal_getfocalRange);
		metaData.scriptClass->AddInternalCall("Internal_setfocalRange", (void*)&ScriptDepthOfFieldSettings::Internal_setfocalRange);
		metaData.scriptClass->AddInternalCall("Internal_getnearTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_getnearTransitionRange);
		metaData.scriptClass->AddInternalCall("Internal_setnearTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_setnearTransitionRange);
		metaData.scriptClass->AddInternalCall("Internal_getfarTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_getfarTransitionRange);
		metaData.scriptClass->AddInternalCall("Internal_setfarTransitionRange", (void*)&ScriptDepthOfFieldSettings::Internal_setfarTransitionRange);
		metaData.scriptClass->AddInternalCall("Internal_getnearBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_getnearBlurAmount);
		metaData.scriptClass->AddInternalCall("Internal_setnearBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_setnearBlurAmount);
		metaData.scriptClass->AddInternalCall("Internal_getfarBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_getfarBlurAmount);
		metaData.scriptClass->AddInternalCall("Internal_setfarBlurAmount", (void*)&ScriptDepthOfFieldSettings::Internal_setfarBlurAmount);
		metaData.scriptClass->AddInternalCall("Internal_getmaxBokehSize", (void*)&ScriptDepthOfFieldSettings::Internal_getmaxBokehSize);
		metaData.scriptClass->AddInternalCall("Internal_setmaxBokehSize", (void*)&ScriptDepthOfFieldSettings::Internal_setmaxBokehSize);
		metaData.scriptClass->AddInternalCall("Internal_getadaptiveColorThreshold", (void*)&ScriptDepthOfFieldSettings::Internal_getadaptiveColorThreshold);
		metaData.scriptClass->AddInternalCall("Internal_setadaptiveColorThreshold", (void*)&ScriptDepthOfFieldSettings::Internal_setadaptiveColorThreshold);
		metaData.scriptClass->AddInternalCall("Internal_getadaptiveRadiusThreshold", (void*)&ScriptDepthOfFieldSettings::Internal_getadaptiveRadiusThreshold);
		metaData.scriptClass->AddInternalCall("Internal_setadaptiveRadiusThreshold", (void*)&ScriptDepthOfFieldSettings::Internal_setadaptiveRadiusThreshold);
		metaData.scriptClass->AddInternalCall("Internal_getapertureSize", (void*)&ScriptDepthOfFieldSettings::Internal_getapertureSize);
		metaData.scriptClass->AddInternalCall("Internal_setapertureSize", (void*)&ScriptDepthOfFieldSettings::Internal_setapertureSize);
		metaData.scriptClass->AddInternalCall("Internal_getfocalLength", (void*)&ScriptDepthOfFieldSettings::Internal_getfocalLength);
		metaData.scriptClass->AddInternalCall("Internal_setfocalLength", (void*)&ScriptDepthOfFieldSettings::Internal_setfocalLength);
		metaData.scriptClass->AddInternalCall("Internal_getsensorSize", (void*)&ScriptDepthOfFieldSettings::Internal_getsensorSize);
		metaData.scriptClass->AddInternalCall("Internal_setsensorSize", (void*)&ScriptDepthOfFieldSettings::Internal_setsensorSize);
		metaData.scriptClass->AddInternalCall("Internal_getbokehOcclusion", (void*)&ScriptDepthOfFieldSettings::Internal_getbokehOcclusion);
		metaData.scriptClass->AddInternalCall("Internal_setbokehOcclusion", (void*)&ScriptDepthOfFieldSettings::Internal_setbokehOcclusion);
		metaData.scriptClass->AddInternalCall("Internal_getocclusionDepthRange", (void*)&ScriptDepthOfFieldSettings::Internal_getocclusionDepthRange);
		metaData.scriptClass->AddInternalCall("Internal_setocclusionDepthRange", (void*)&ScriptDepthOfFieldSettings::Internal_setocclusionDepthRange);

	}

	MonoObject* ScriptDepthOfFieldSettings::create(const SPtr<DepthOfFieldSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptDepthOfFieldSettings>()) ScriptDepthOfFieldSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptDepthOfFieldSettings::Internal_DepthOfFieldSettings(MonoObject* managedInstance)
	{
		SPtr<DepthOfFieldSettings> instance = bs_shared_ptr_new<DepthOfFieldSettings>();
		new (bs_alloc<ScriptDepthOfFieldSettings>())ScriptDepthOfFieldSettings(managedInstance, instance);
	}

	MonoObject* ScriptDepthOfFieldSettings::Internal_getbokehShape(ScriptDepthOfFieldSettings* thisPtr)
	{
		ResourceHandle<Texture> tmp__output;
		tmp__output = thisPtr->GetInternal()->bokehShape;

		MonoObject* __output;
		ScriptRRefBase* script__output;
		script__output = ScriptResourceManager::instance().GetScriptRRef(tmp__output);
		if(script__output != nullptr)
			__output = script__output->GetManagedInstance();
		else
			__output = nullptr;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setbokehShape(ScriptDepthOfFieldSettings* thisPtr, MonoObject* value)
	{
		ResourceHandle<Texture> tmpvalue;
		ScriptRRefBase* scriptvalue;
		scriptvalue = ScriptRRefBase::toNative(value);
		if(scriptvalue != nullptr)
			tmpvalue = static_resource_cast<Texture>(scriptvalue->GetHandle());
		thisPtr->GetInternal()->bokehShape = tmpvalue;
	}

	bool ScriptDepthOfFieldSettings::Internal_getenabled(ScriptDepthOfFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setenabled(ScriptDepthOfFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	DepthOfFieldType ScriptDepthOfFieldSettings::Internal_gettype(ScriptDepthOfFieldSettings* thisPtr)
	{
		DepthOfFieldType tmp__output;
		tmp__output = thisPtr->GetInternal()->type;

		DepthOfFieldType __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_settype(ScriptDepthOfFieldSettings* thisPtr, DepthOfFieldType value)
	{
		thisPtr->GetInternal()->type = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfocalDistance(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->focalDistance;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfocalDistance(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->focalDistance = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfocalRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->focalRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfocalRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->focalRange = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getnearTransitionRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->nearTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setnearTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->nearTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfarTransitionRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->farTransitionRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfarTransitionRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->farTransitionRange = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getnearBlurAmount(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->nearBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setnearBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->nearBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfarBlurAmount(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->farBlurAmount;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfarBlurAmount(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->farBlurAmount = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getmaxBokehSize(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->maxBokehSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setmaxBokehSize(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->maxBokehSize = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getadaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->adaptiveColorThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setadaptiveColorThreshold(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->adaptiveColorThreshold = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getadaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->adaptiveRadiusThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setadaptiveRadiusThreshold(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->adaptiveRadiusThreshold = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getapertureSize(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->apertureSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setapertureSize(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->apertureSize = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getfocalLength(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->focalLength;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setfocalLength(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->focalLength = value;
	}

	void ScriptDepthOfFieldSettings::Internal_getsensorSize(ScriptDepthOfFieldSettings* thisPtr, Vector2* __output)
	{
		Vector2 tmp__output;
		tmp__output = thisPtr->GetInternal()->sensorSize;

		*__output = tmp__output;


	}

	void ScriptDepthOfFieldSettings::Internal_setsensorSize(ScriptDepthOfFieldSettings* thisPtr, Vector2* value)
	{
		thisPtr->GetInternal()->sensorSize = *value;
	}

	bool ScriptDepthOfFieldSettings::Internal_getbokehOcclusion(ScriptDepthOfFieldSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->bokehOcclusion;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setbokehOcclusion(ScriptDepthOfFieldSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->bokehOcclusion = value;
	}

	float ScriptDepthOfFieldSettings::Internal_getocclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->occlusionDepthRange;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptDepthOfFieldSettings::Internal_setocclusionDepthRange(ScriptDepthOfFieldSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->occlusionDepthRange = value;
	}
}
