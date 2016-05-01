//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPostProcessSettings.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsCoreThread.h"

namespace BansheeEngine
{
	ScriptPostProcessSettings::ScriptPostProcessSettings(MonoObject* instance)
		:ScriptObject(instance), mSettings(bs_shared_ptr_new<PostProcessSettings>())
	{ }

	void ScriptPostProcessSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptPostProcessSettings::internal_createInstance);

		metaData.scriptClass->addInternalCall("Internal_GetEnableAutoExposure", &ScriptPostProcessSettings::internal_GetEnableAutoExposure);
		metaData.scriptClass->addInternalCall("Internal_SetEnableAutoExposure", &ScriptPostProcessSettings::internal_SetEnableAutoExposure);

		metaData.scriptClass->addInternalCall("Internal_GetAutoExposureSettings", &ScriptPostProcessSettings::internal_GetAutoExposureSettings);
		metaData.scriptClass->addInternalCall("Internal_SetAutoExposureSettings", &ScriptPostProcessSettings::internal_SetAutoExposureSettings);

		metaData.scriptClass->addInternalCall("Internal_GetEnableTonemapping", &ScriptPostProcessSettings::internal_GetEnableTonemapping);
		metaData.scriptClass->addInternalCall("Internal_SetEnableTonemapping", &ScriptPostProcessSettings::internal_SetEnableTonemapping);

		metaData.scriptClass->addInternalCall("Internal_GetTonemappingSettings", &ScriptPostProcessSettings::internal_GetTonemappingSettings);
		metaData.scriptClass->addInternalCall("Internal_SetTonemappingSettings", &ScriptPostProcessSettings::internal_SetTonemappingSettings);

		metaData.scriptClass->addInternalCall("Internal_GetWhiteBalanceSettings", &ScriptPostProcessSettings::internal_GetWhiteBalanceSettings);
		metaData.scriptClass->addInternalCall("Internal_SetWhiteBalanceSettings", &ScriptPostProcessSettings::internal_SetWhiteBalanceSettings);

		metaData.scriptClass->addInternalCall("Internal_GetColorGradingSettings", &ScriptPostProcessSettings::internal_GetColorGradingSettings);
		metaData.scriptClass->addInternalCall("Internal_SetColorGradingSettings", &ScriptPostProcessSettings::internal_SetColorGradingSettings);

		metaData.scriptClass->addInternalCall("Internal_GetExposureScale", &ScriptPostProcessSettings::internal_GetExposureScale);
		metaData.scriptClass->addInternalCall("Internal_SetExposureScale", &ScriptPostProcessSettings::internal_SetExposureScale);

		metaData.scriptClass->addInternalCall("Internal_GetGamma", &ScriptPostProcessSettings::internal_GetGamma);
		metaData.scriptClass->addInternalCall("Internal_SetGamma", &ScriptPostProcessSettings::internal_SetGamma);

		// Validation
#if BS_DEBUG_MODE
		MonoAssembly* engineAssembly = MonoManager::instance().getAssembly(ENGINE_ASSEMBLY);

		MonoClass* autoExposureSettingsClass = engineAssembly->getClass("BansheeEngine", "AutoExposureSettings");
		MonoClass* tonemappingSettingsClass = engineAssembly->getClass("BansheeEngine", "TonemappingSettings");
		MonoClass* whieBalanceSettingsClass = engineAssembly->getClass("BansheeEngine", "WhiteBalanceSettings");
		MonoClass* colorGradingSettingsClass = engineAssembly->getClass("BansheeEngine", "ColorGradingSettings");

		assert(autoExposureSettingsClass->getInstanceSize() == sizeof(AutoExposureSettings));
		assert(tonemappingSettingsClass->getInstanceSize() == sizeof(TonemappingSettings));
		assert(whieBalanceSettingsClass->getInstanceSize() == sizeof(WhiteBalanceSettings));
		assert(colorGradingSettingsClass->getInstanceSize() == sizeof(ColorGradingSettings));
#endif
	}

	void ScriptPostProcessSettings::internal_createInstance(MonoObject* managedInstance)
	{
		new (bs_alloc<ScriptPostProcessSettings>()) ScriptPostProcessSettings(managedInstance);
	}

	bool ScriptPostProcessSettings::internal_GetEnableAutoExposure(ScriptPostProcessSettings* thisPtr)
	{
		return thisPtr->mSettings->enableAutoExposure;
	}

	void ScriptPostProcessSettings::internal_SetEnableAutoExposure(ScriptPostProcessSettings* thisPtr, bool value)
	{
		thisPtr->mSettings->enableAutoExposure = value;
	}

	void ScriptPostProcessSettings::internal_GetAutoExposureSettings(ScriptPostProcessSettings* thisPtr, AutoExposureSettings* value)
	{
		*value = thisPtr->mSettings->autoExposure;
	}

	void ScriptPostProcessSettings::internal_SetAutoExposureSettings(ScriptPostProcessSettings* thisPtr, AutoExposureSettings* value)
	{
		thisPtr->mSettings->autoExposure = *value;
	}

	bool ScriptPostProcessSettings::internal_GetEnableTonemapping(ScriptPostProcessSettings* thisPtr)
	{
		return thisPtr->mSettings->enableTonemapping;
	}

	void ScriptPostProcessSettings::internal_SetEnableTonemapping(ScriptPostProcessSettings* thisPtr, bool value)
	{
		thisPtr->mSettings->enableTonemapping = value;
	}

	void ScriptPostProcessSettings::internal_GetTonemappingSettings(ScriptPostProcessSettings* thisPtr, TonemappingSettings* value)
	{
		*value = thisPtr->mSettings->tonemapping;
	}

	void ScriptPostProcessSettings::internal_SetTonemappingSettings(ScriptPostProcessSettings* thisPtr, TonemappingSettings* value)
	{
		thisPtr->mSettings->tonemapping = *value;
	}

	void ScriptPostProcessSettings::internal_GetWhiteBalanceSettings(ScriptPostProcessSettings* thisPtr, WhiteBalanceSettings* value)
	{
		*value = thisPtr->mSettings->whiteBalance;
	}

	void ScriptPostProcessSettings::internal_SetWhiteBalanceSettings(ScriptPostProcessSettings* thisPtr, WhiteBalanceSettings* value)
	{
		thisPtr->mSettings->whiteBalance = *value;
	}

	void ScriptPostProcessSettings::internal_GetColorGradingSettings(ScriptPostProcessSettings* thisPtr, ColorGradingSettings* value)
	{
		*value = thisPtr->mSettings->colorGrading;
	}

	void ScriptPostProcessSettings::internal_SetColorGradingSettings(ScriptPostProcessSettings* thisPtr, ColorGradingSettings* value)
	{
		thisPtr->mSettings->colorGrading = *value;
	}

	float ScriptPostProcessSettings::internal_GetExposureScale(ScriptPostProcessSettings* thisPtr)
	{
		return thisPtr->mSettings->exposureScale;
	}

	void ScriptPostProcessSettings::internal_SetExposureScale(ScriptPostProcessSettings* thisPtr, float value)
	{
		thisPtr->mSettings->exposureScale = value;
	}

	float ScriptPostProcessSettings::internal_GetGamma(ScriptPostProcessSettings* thisPtr)
	{
		return thisPtr->mSettings->gamma;
	}

	void ScriptPostProcessSettings::internal_SetGamma(ScriptPostProcessSettings* thisPtr, float value)
	{
		thisPtr->mSettings->gamma = value;
	}
}