//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptScreenSpaceLensFlareSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptScreenSpaceLensFlareSettings::ScriptScreenSpaceLensFlareSettings(MonoObject* managedInstance, const SPtr<ScreenSpaceLensFlareSettings>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptScreenSpaceLensFlareSettings::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_ScreenSpaceLensFlareSettings", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_ScreenSpaceLensFlareSettings);
		metaData.scriptClass->AddInternalCall("Internal_getenabled", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getenabled);
		metaData.scriptClass->AddInternalCall("Internal_setenabled", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setenabled);
		metaData.scriptClass->AddInternalCall("Internal_getdownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getdownsampleCount);
		metaData.scriptClass->AddInternalCall("Internal_setdownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setdownsampleCount);
		metaData.scriptClass->AddInternalCall("Internal_getthreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getthreshold);
		metaData.scriptClass->AddInternalCall("Internal_setthreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setthreshold);
		metaData.scriptClass->AddInternalCall("Internal_getghostCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getghostCount);
		metaData.scriptClass->AddInternalCall("Internal_setghostCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setghostCount);
		metaData.scriptClass->AddInternalCall("Internal_getghostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getghostSpacing);
		metaData.scriptClass->AddInternalCall("Internal_setghostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setghostSpacing);
		metaData.scriptClass->AddInternalCall("Internal_getbrightness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getbrightness);
		metaData.scriptClass->AddInternalCall("Internal_setbrightness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setbrightness);
		metaData.scriptClass->AddInternalCall("Internal_getfilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getfilterSize);
		metaData.scriptClass->AddInternalCall("Internal_setfilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setfilterSize);
		metaData.scriptClass->AddInternalCall("Internal_gethalo", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethalo);
		metaData.scriptClass->AddInternalCall("Internal_sethalo", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethalo);
		metaData.scriptClass->AddInternalCall("Internal_gethaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloRadius);
		metaData.scriptClass->AddInternalCall("Internal_sethaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloRadius);
		metaData.scriptClass->AddInternalCall("Internal_gethaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloThickness);
		metaData.scriptClass->AddInternalCall("Internal_sethaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloThickness);
		metaData.scriptClass->AddInternalCall("Internal_gethaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloThreshold);
		metaData.scriptClass->AddInternalCall("Internal_sethaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloThreshold);
		metaData.scriptClass->AddInternalCall("Internal_gethaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloAspectRatio);
		metaData.scriptClass->AddInternalCall("Internal_sethaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloAspectRatio);
		metaData.scriptClass->AddInternalCall("Internal_getchromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberration);
		metaData.scriptClass->AddInternalCall("Internal_setchromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberration);
		metaData.scriptClass->AddInternalCall("Internal_getchromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberrationOffset);
		metaData.scriptClass->AddInternalCall("Internal_setchromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberrationOffset);
		metaData.scriptClass->AddInternalCall("Internal_getbicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getbicubicUpsampling);
		metaData.scriptClass->AddInternalCall("Internal_setbicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setbicubicUpsampling);

	}

	MonoObject* ScriptScreenSpaceLensFlareSettings::create(const SPtr<ScreenSpaceLensFlareSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptScreenSpaceLensFlareSettings>()) ScriptScreenSpaceLensFlareSettings(managedInstance, value);
		return managedInstance;
	}
	void ScriptScreenSpaceLensFlareSettings::Internal_ScreenSpaceLensFlareSettings(MonoObject* managedInstance)
	{
		SPtr<ScreenSpaceLensFlareSettings> instance = bs_shared_ptr_new<ScreenSpaceLensFlareSettings>();
		new (bs_alloc<ScriptScreenSpaceLensFlareSettings>())ScriptScreenSpaceLensFlareSettings(managedInstance, instance);
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_getenabled(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setenabled(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->enabled = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::Internal_getdownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->downsampleCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setdownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->downsampleCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getthreshold(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setthreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->threshold = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::Internal_getghostCount(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->ghostCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setghostCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->ghostCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getghostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->ghostSpacing;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setghostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->ghostSpacing = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getbrightness(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->brightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setbrightness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->brightness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getfilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->filterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setfilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->filterSize = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_gethalo(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->halo;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethalo(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->halo = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->haloRadius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->haloRadius = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->haloThickness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->haloThickness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->haloThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->haloThreshold = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->haloAspectRatio;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->haloAspectRatio = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->chromaticAberration;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->chromaticAberration = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->GetInternal()->chromaticAberrationOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->GetInternal()->chromaticAberrationOffset = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_getbicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->bicubicUpsampling;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setbicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->GetInternal()->bicubicUpsampling = value;
	}
}
