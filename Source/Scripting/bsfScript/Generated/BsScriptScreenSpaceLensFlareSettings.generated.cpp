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

	void ScriptScreenSpaceLensFlareSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_ScreenSpaceLensFlareSettings", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_ScreenSpaceLensFlareSettings);
		metaData.scriptClass->addInternalCall("Internal_getenabled", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getenabled);
		metaData.scriptClass->addInternalCall("Internal_setenabled", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setenabled);
		metaData.scriptClass->addInternalCall("Internal_getdownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getdownsampleCount);
		metaData.scriptClass->addInternalCall("Internal_setdownsampleCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setdownsampleCount);
		metaData.scriptClass->addInternalCall("Internal_getthreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getthreshold);
		metaData.scriptClass->addInternalCall("Internal_setthreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setthreshold);
		metaData.scriptClass->addInternalCall("Internal_getghostCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getghostCount);
		metaData.scriptClass->addInternalCall("Internal_setghostCount", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setghostCount);
		metaData.scriptClass->addInternalCall("Internal_getghostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getghostSpacing);
		metaData.scriptClass->addInternalCall("Internal_setghostSpacing", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setghostSpacing);
		metaData.scriptClass->addInternalCall("Internal_getbrightness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getbrightness);
		metaData.scriptClass->addInternalCall("Internal_setbrightness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setbrightness);
		metaData.scriptClass->addInternalCall("Internal_getfilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getfilterSize);
		metaData.scriptClass->addInternalCall("Internal_setfilterSize", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setfilterSize);
		metaData.scriptClass->addInternalCall("Internal_gethalo", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethalo);
		metaData.scriptClass->addInternalCall("Internal_sethalo", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethalo);
		metaData.scriptClass->addInternalCall("Internal_gethaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloRadius);
		metaData.scriptClass->addInternalCall("Internal_sethaloRadius", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloRadius);
		metaData.scriptClass->addInternalCall("Internal_gethaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloThickness);
		metaData.scriptClass->addInternalCall("Internal_sethaloThickness", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloThickness);
		metaData.scriptClass->addInternalCall("Internal_gethaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloThreshold);
		metaData.scriptClass->addInternalCall("Internal_sethaloThreshold", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloThreshold);
		metaData.scriptClass->addInternalCall("Internal_gethaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_gethaloAspectRatio);
		metaData.scriptClass->addInternalCall("Internal_sethaloAspectRatio", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_sethaloAspectRatio);
		metaData.scriptClass->addInternalCall("Internal_getchromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberration);
		metaData.scriptClass->addInternalCall("Internal_setchromaticAberration", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberration);
		metaData.scriptClass->addInternalCall("Internal_getchromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberrationOffset);
		metaData.scriptClass->addInternalCall("Internal_setchromaticAberrationOffset", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberrationOffset);
		metaData.scriptClass->addInternalCall("Internal_getbicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_getbicubicUpsampling);
		metaData.scriptClass->addInternalCall("Internal_setbicubicUpsampling", (void*)&ScriptScreenSpaceLensFlareSettings::Internal_setbicubicUpsampling);

	}

	MonoObject* ScriptScreenSpaceLensFlareSettings::create(const SPtr<ScreenSpaceLensFlareSettings>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->createInstance("bool", ctorParams);
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
		tmp__output = thisPtr->getInternal()->enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setenabled(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->enabled = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::Internal_getdownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->downsampleCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setdownsampleCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->downsampleCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getthreshold(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->threshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setthreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->threshold = value;
	}

	uint32_t ScriptScreenSpaceLensFlareSettings::Internal_getghostCount(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->getInternal()->ghostCount;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setghostCount(ScriptScreenSpaceLensFlareSettings* thisPtr, uint32_t value)
	{
		thisPtr->getInternal()->ghostCount = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getghostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->ghostSpacing;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setghostSpacing(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->ghostSpacing = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getbrightness(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->brightness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setbrightness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->brightness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getfilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->filterSize;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setfilterSize(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->filterSize = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_gethalo(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->halo;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethalo(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->halo = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->haloRadius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloRadius(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->haloRadius = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->haloThickness;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloThickness(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->haloThickness = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->haloThreshold;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloThreshold(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->haloThreshold = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_gethaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->haloAspectRatio;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_sethaloAspectRatio(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->haloAspectRatio = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->chromaticAberration;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberration(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->chromaticAberration = value;
	}

	float ScriptScreenSpaceLensFlareSettings::Internal_getchromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		float tmp__output;
		tmp__output = thisPtr->getInternal()->chromaticAberrationOffset;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setchromaticAberrationOffset(ScriptScreenSpaceLensFlareSettings* thisPtr, float value)
	{
		thisPtr->getInternal()->chromaticAberrationOffset = value;
	}

	bool ScriptScreenSpaceLensFlareSettings::Internal_getbicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->getInternal()->bicubicUpsampling;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptScreenSpaceLensFlareSettings::Internal_setbicubicUpsampling(ScriptScreenSpaceLensFlareSettings* thisPtr, bool value)
	{
		thisPtr->getInternal()->bicubicUpsampling = value;
	}
}
