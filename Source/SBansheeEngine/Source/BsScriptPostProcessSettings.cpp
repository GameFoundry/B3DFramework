//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptPostProcessSettings.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsCoreThread.h"
#include "BsStandardPostProcessSettings.h"

namespace bs
{
	MonoField* ScriptAutoExposureSettings::sHistogramLog2Min = nullptr;
	MonoField* ScriptAutoExposureSettings::sHistogramLog2Max = nullptr;
	MonoField* ScriptAutoExposureSettings::sHistogramPctLow = nullptr;
	MonoField* ScriptAutoExposureSettings::sHistogramPctHigh = nullptr;
	MonoField* ScriptAutoExposureSettings::sMinEyeAdaptation = nullptr;
	MonoField* ScriptAutoExposureSettings::sMaxEyeAdaptation = nullptr;
	MonoField* ScriptAutoExposureSettings::sEyeAdaptationSpeedUp = nullptr;
	MonoField* ScriptAutoExposureSettings::sEyeAdaptationSpeedDown = nullptr;

	ScriptAutoExposureSettings::ScriptAutoExposureSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptAutoExposureSettings::initRuntimeData()
	{
		sHistogramLog2Min = metaData.scriptClass->getField("HistogramLog2Min");
		sHistogramLog2Max = metaData.scriptClass->getField("HistogramLog2Max");
		sHistogramPctLow = metaData.scriptClass->getField("HistogramPctLow");
		sHistogramPctHigh = metaData.scriptClass->getField("HistogramPctHigh");
		sMinEyeAdaptation = metaData.scriptClass->getField("MinEyeAdaptation");
		sMaxEyeAdaptation = metaData.scriptClass->getField("MaxEyeAdaptation");
		sEyeAdaptationSpeedUp = metaData.scriptClass->getField("EyeAdaptationSpeedUp");
		sEyeAdaptationSpeedDown = metaData.scriptClass->getField("EyeAdaptationSpeedDown");
	}

	SPtr<AutoExposureSettings> ScriptAutoExposureSettings::toNative(MonoObject* object)
	{
		SPtr<AutoExposureSettings> output = bs_shared_ptr_new<AutoExposureSettings>();

		sHistogramLog2Min->getValue(object, &output->histogramLog2Min);
		sHistogramLog2Max->getValue(object, &output->histogramLog2Max);
		sHistogramPctLow->getValue(object, &output->histogramPctLow);
		sHistogramPctHigh->getValue(object, &output->histogramPctHigh);
		sMinEyeAdaptation->getValue(object, &output->minEyeAdaptation);
		sMaxEyeAdaptation->getValue(object, &output->maxEyeAdaptation);
		sEyeAdaptationSpeedUp->getValue(object, &output->eyeAdaptationSpeedUp);
		sEyeAdaptationSpeedDown->getValue(object, &output->eyeAdaptationSpeedDown);

		return output;
	}

	MonoObject* ScriptAutoExposureSettings::toManaged(AutoExposureSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sHistogramLog2Min->setValue(object, &value.histogramLog2Min);
		sHistogramLog2Max->setValue(object, &value.histogramLog2Max);
		sHistogramPctLow->setValue(object, &value.histogramPctLow);
		sHistogramPctHigh->setValue(object, &value.histogramPctHigh);
		sMinEyeAdaptation->setValue(object, &value.minEyeAdaptation);
		sMaxEyeAdaptation->setValue(object, &value.maxEyeAdaptation);
		sEyeAdaptationSpeedUp->setValue(object, &value.eyeAdaptationSpeedUp);
		sEyeAdaptationSpeedDown->setValue(object, &value.eyeAdaptationSpeedDown);

		return object;
	}

	MonoField* ScriptTonemappingSettings::sFilmicCurveShoulderStrength = nullptr;
	MonoField* ScriptTonemappingSettings::sFilmicCurveLinearStrength = nullptr;
	MonoField* ScriptTonemappingSettings::sFilmicCurveLinearAngle = nullptr;
	MonoField* ScriptTonemappingSettings::sFilmicCurveToeStrength = nullptr;
	MonoField* ScriptTonemappingSettings::sFilmicCurveToeNumerator = nullptr;
	MonoField* ScriptTonemappingSettings::sFilmicCurveToeDenominator = nullptr;
	MonoField* ScriptTonemappingSettings::sFilmicCurveLinearWhitePoint = nullptr;

	ScriptTonemappingSettings::ScriptTonemappingSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptTonemappingSettings::initRuntimeData()
	{
		sFilmicCurveShoulderStrength = metaData.scriptClass->getField("FilmicCurveShoulderStrength");
		sFilmicCurveLinearStrength = metaData.scriptClass->getField("FilmicCurveLinearStrength");
		sFilmicCurveLinearAngle = metaData.scriptClass->getField("FilmicCurveLinearAngle");
		sFilmicCurveToeStrength = metaData.scriptClass->getField("FilmicCurveToeStrength");
		sFilmicCurveToeNumerator = metaData.scriptClass->getField("FilmicCurveToeNumerator");
		sFilmicCurveToeDenominator = metaData.scriptClass->getField("FilmicCurveToeDenominator");
		sFilmicCurveLinearWhitePoint = metaData.scriptClass->getField("FilmicCurveLinearWhitePoint");
	}

	SPtr<TonemappingSettings> ScriptTonemappingSettings::toNative(MonoObject* object)
	{
		SPtr<TonemappingSettings> output = bs_shared_ptr_new<TonemappingSettings>();

		sFilmicCurveShoulderStrength->getValue(object, &output->filmicCurveShoulderStrength);
		sFilmicCurveLinearStrength->getValue(object, &output->filmicCurveLinearStrength);
		sFilmicCurveLinearAngle->getValue(object, &output->filmicCurveLinearAngle);
		sFilmicCurveToeStrength->getValue(object, &output->filmicCurveToeStrength);
		sFilmicCurveToeNumerator->getValue(object, &output->filmicCurveToeNumerator);
		sFilmicCurveToeDenominator->getValue(object, &output->filmicCurveToeDenominator);
		sFilmicCurveLinearWhitePoint->getValue(object, &output->filmicCurveLinearWhitePoint);

		return output;
	}

	MonoObject* ScriptTonemappingSettings::toManaged(TonemappingSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sFilmicCurveShoulderStrength->setValue(object, &value.filmicCurveShoulderStrength);
		sFilmicCurveLinearStrength->setValue(object, &value.filmicCurveLinearStrength);
		sFilmicCurveLinearAngle->setValue(object, &value.filmicCurveLinearAngle);
		sFilmicCurveToeStrength->setValue(object, &value.filmicCurveToeStrength);
		sFilmicCurveToeNumerator->setValue(object, &value.filmicCurveToeNumerator);
		sFilmicCurveToeDenominator->setValue(object, &value.filmicCurveToeDenominator);
		sFilmicCurveLinearWhitePoint->setValue(object, &value.filmicCurveLinearWhitePoint);

		return object;
	}

	MonoField* ScriptWhiteBalanceSettings::sTemperature = nullptr;
	MonoField* ScriptWhiteBalanceSettings::sTint = nullptr;

	ScriptWhiteBalanceSettings::ScriptWhiteBalanceSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptWhiteBalanceSettings::initRuntimeData()
	{
		sTemperature = metaData.scriptClass->getField("Temperature");
		sTint = metaData.scriptClass->getField("Tint");
	}

	SPtr<WhiteBalanceSettings> ScriptWhiteBalanceSettings::toNative(MonoObject* object)
	{
		SPtr<WhiteBalanceSettings> output = bs_shared_ptr_new<WhiteBalanceSettings>();

		sTemperature->getValue(object, &output->temperature);
		sTint->getValue(object, &output->tint);

		return output;
	}

	MonoObject* ScriptWhiteBalanceSettings::toManaged(WhiteBalanceSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sTemperature->setValue(object, &value.temperature);
		sTint->setValue(object, &value.tint);

		return object;
	}

	MonoField* ScriptColorGradingSettings::sSaturation = nullptr;
	MonoField* ScriptColorGradingSettings::sContrast = nullptr;
	MonoField* ScriptColorGradingSettings::sGain = nullptr;
	MonoField* ScriptColorGradingSettings::sOffset = nullptr;

	ScriptColorGradingSettings::ScriptColorGradingSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptColorGradingSettings::initRuntimeData()
	{
		sSaturation = metaData.scriptClass->getField("Saturation");
		sContrast = metaData.scriptClass->getField("Contrast");
		sGain = metaData.scriptClass->getField("Gain");
		sOffset = metaData.scriptClass->getField("Offset");
	}

	SPtr<ColorGradingSettings> ScriptColorGradingSettings::toNative(MonoObject* object)
	{
		SPtr<ColorGradingSettings> output = bs_shared_ptr_new<ColorGradingSettings>();

		sSaturation->getValue(object, &output->saturation);
		sContrast->getValue(object, &output->contrast);
		sGain->getValue(object, &output->gain);
		sOffset->getValue(object, &output->offset);

		return output;
	}

	MonoObject* ScriptColorGradingSettings::toManaged(ColorGradingSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sSaturation->setValue(object, &value.saturation);
		sContrast->setValue(object, &value.contrast);
		sGain->setValue(object, &value.gain);
		sOffset->setValue(object, &value.offset);

		return object;
	}

	MonoField* ScriptPostProcessSettings::sEnableAutoExposure = nullptr;
	MonoField* ScriptPostProcessSettings::sAutoExposure = nullptr;
	MonoField* ScriptPostProcessSettings::sEnableTonemapping = nullptr;
	MonoField* ScriptPostProcessSettings::sTonemapping = nullptr;
	MonoField* ScriptPostProcessSettings::sWhiteBalance = nullptr;
	MonoField* ScriptPostProcessSettings::sColorGrading = nullptr;
	MonoField* ScriptPostProcessSettings::sExposureScale = nullptr;
	MonoField* ScriptPostProcessSettings::sGamma = nullptr;

	ScriptPostProcessSettings::ScriptPostProcessSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptPostProcessSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateDefault", &ScriptPostProcessSettings::internal_CreateDefault);

		sEnableAutoExposure = metaData.scriptClass->getField("EnableAutoExposure");
		sAutoExposure = metaData.scriptClass->getField("AutoExposure");
		sEnableTonemapping = metaData.scriptClass->getField("EnableTonemapping");
		sTonemapping = metaData.scriptClass->getField("Tonemapping");
		sWhiteBalance = metaData.scriptClass->getField("WhiteBalance");
		sColorGrading = metaData.scriptClass->getField("ColorGrading");
		sExposureScale = metaData.scriptClass->getField("ExposureScale");
		sGamma = metaData.scriptClass->getField("Gamma");
	}

	SPtr<StandardPostProcessSettings> ScriptPostProcessSettings::toNative(MonoObject* object)
	{
		SPtr<StandardPostProcessSettings> output = bs_shared_ptr_new<StandardPostProcessSettings>();

		sEnableAutoExposure->getValue(object, &output->enableAutoExposure);
		sEnableTonemapping->getValue(object, &output->enableTonemapping);
		sExposureScale->getValue(object, &output->exposureScale);
		sGamma->getValue(object, &output->gamma);

		MonoObject* autoExposureMono;
		sAutoExposure->getValue(object, &autoExposureMono);
		output->autoExposure = *ScriptAutoExposureSettings::toNative(autoExposureMono);

		MonoObject* tonemappingMono;
		sTonemapping->getValue(object, &tonemappingMono);
		output->tonemapping = *ScriptTonemappingSettings::toNative(tonemappingMono);

		MonoObject* whiteBalanceMono;
		sWhiteBalance->getValue(object, &whiteBalanceMono);
		output->whiteBalance = *ScriptWhiteBalanceSettings::toNative(whiteBalanceMono);

		MonoObject* colorGradingMono;
		sColorGrading->getValue(object, &colorGradingMono);
		output->colorGrading = *ScriptColorGradingSettings::toNative(colorGradingMono);

		return output;
	}

	MonoObject* ScriptPostProcessSettings::toManaged(const SPtr<StandardPostProcessSettings>& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sEnableAutoExposure->setValue(object, &value->enableAutoExposure);
		sEnableTonemapping->setValue(object, &value->enableTonemapping);
		sExposureScale->setValue(object, &value->exposureScale);
		sGamma->setValue(object, &value->gamma);

		MonoObject* autoExposureMono = ScriptAutoExposureSettings::toManaged(value->autoExposure);
		sAutoExposure->setValue(object, autoExposureMono);

		MonoObject* tonemappingMono = ScriptTonemappingSettings::toManaged(value->tonemapping);
		sTonemapping->setValue(object, tonemappingMono);

		MonoObject* whiteBalanceMono = ScriptWhiteBalanceSettings::toManaged(value->whiteBalance);
		sWhiteBalance->setValue(object, whiteBalanceMono);

		MonoObject* colorGradingMono = ScriptColorGradingSettings::toManaged(value->colorGrading);
		sColorGrading->setValue(object, colorGradingMono);

		return object;
	}

	MonoObject* ScriptPostProcessSettings::internal_CreateDefault()
	{
		return toManaged(bs_shared_ptr_new<StandardPostProcessSettings>());
	}
}