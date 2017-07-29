//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptRenderSettings.h"
#include "BsMonoManager.h"
#include "BsMonoAssembly.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "BsCoreThread.h"
#include "BsRenderSettings.h"

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

		sHistogramLog2Min->get(object, &output->histogramLog2Min);
		sHistogramLog2Max->get(object, &output->histogramLog2Max);
		sHistogramPctLow->get(object, &output->histogramPctLow);
		sHistogramPctHigh->get(object, &output->histogramPctHigh);
		sMinEyeAdaptation->get(object, &output->minEyeAdaptation);
		sMaxEyeAdaptation->get(object, &output->maxEyeAdaptation);
		sEyeAdaptationSpeedUp->get(object, &output->eyeAdaptationSpeedUp);
		sEyeAdaptationSpeedDown->get(object, &output->eyeAdaptationSpeedDown);

		return output;
	}

	MonoObject* ScriptAutoExposureSettings::toManaged(AutoExposureSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sHistogramLog2Min->set(object, &value.histogramLog2Min);
		sHistogramLog2Max->set(object, &value.histogramLog2Max);
		sHistogramPctLow->set(object, &value.histogramPctLow);
		sHistogramPctHigh->set(object, &value.histogramPctHigh);
		sMinEyeAdaptation->set(object, &value.minEyeAdaptation);
		sMaxEyeAdaptation->set(object, &value.maxEyeAdaptation);
		sEyeAdaptationSpeedUp->set(object, &value.eyeAdaptationSpeedUp);
		sEyeAdaptationSpeedDown->set(object, &value.eyeAdaptationSpeedDown);

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

		sFilmicCurveShoulderStrength->get(object, &output->filmicCurveShoulderStrength);
		sFilmicCurveLinearStrength->get(object, &output->filmicCurveLinearStrength);
		sFilmicCurveLinearAngle->get(object, &output->filmicCurveLinearAngle);
		sFilmicCurveToeStrength->get(object, &output->filmicCurveToeStrength);
		sFilmicCurveToeNumerator->get(object, &output->filmicCurveToeNumerator);
		sFilmicCurveToeDenominator->get(object, &output->filmicCurveToeDenominator);
		sFilmicCurveLinearWhitePoint->get(object, &output->filmicCurveLinearWhitePoint);

		return output;
	}

	MonoObject* ScriptTonemappingSettings::toManaged(TonemappingSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sFilmicCurveShoulderStrength->set(object, &value.filmicCurveShoulderStrength);
		sFilmicCurveLinearStrength->set(object, &value.filmicCurveLinearStrength);
		sFilmicCurveLinearAngle->set(object, &value.filmicCurveLinearAngle);
		sFilmicCurveToeStrength->set(object, &value.filmicCurveToeStrength);
		sFilmicCurveToeNumerator->set(object, &value.filmicCurveToeNumerator);
		sFilmicCurveToeDenominator->set(object, &value.filmicCurveToeDenominator);
		sFilmicCurveLinearWhitePoint->set(object, &value.filmicCurveLinearWhitePoint);

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

		sTemperature->get(object, &output->temperature);
		sTint->get(object, &output->tint);

		return output;
	}

	MonoObject* ScriptWhiteBalanceSettings::toManaged(WhiteBalanceSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sTemperature->set(object, &value.temperature);
		sTint->set(object, &value.tint);

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

		sSaturation->get(object, &output->saturation);
		sContrast->get(object, &output->contrast);
		sGain->get(object, &output->gain);
		sOffset->get(object, &output->offset);

		return output;
	}

	MonoObject* ScriptColorGradingSettings::toManaged(ColorGradingSettings& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sSaturation->set(object, &value.saturation);
		sContrast->set(object, &value.contrast);
		sGain->set(object, &value.gain);
		sOffset->set(object, &value.offset);

		return object;
	}

	MonoField* ScriptRenderSettings::sEnableAutoExposure = nullptr;
	MonoField* ScriptRenderSettings::sAutoExposure = nullptr;
	MonoField* ScriptRenderSettings::sEnableTonemapping = nullptr;
	MonoField* ScriptRenderSettings::sTonemapping = nullptr;
	MonoField* ScriptRenderSettings::sWhiteBalance = nullptr;
	MonoField* ScriptRenderSettings::sColorGrading = nullptr;
	MonoField* ScriptRenderSettings::sExposureScale = nullptr;
	MonoField* ScriptRenderSettings::sGamma = nullptr;
	MonoField* ScriptRenderSettings::sEnableHDR = nullptr;
	MonoField* ScriptRenderSettings::sEnableLighting = nullptr;
	MonoField* ScriptRenderSettings::sEnableShadows = nullptr;
	MonoField* ScriptRenderSettings::sOverlayOnly = nullptr;

	ScriptRenderSettings::ScriptRenderSettings(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptRenderSettings::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateDefault", &ScriptRenderSettings::internal_CreateDefault);

		sEnableAutoExposure = metaData.scriptClass->getField("EnableAutoExposure");
		sAutoExposure = metaData.scriptClass->getField("AutoExposure");
		sEnableTonemapping = metaData.scriptClass->getField("EnableTonemapping");
		sTonemapping = metaData.scriptClass->getField("Tonemapping");
		sWhiteBalance = metaData.scriptClass->getField("WhiteBalance");
		sColorGrading = metaData.scriptClass->getField("ColorGrading");
		sExposureScale = metaData.scriptClass->getField("ExposureScale");
		sGamma = metaData.scriptClass->getField("Gamma");
		sEnableHDR = metaData.scriptClass->getField("EnableHDR");
		sEnableLighting = metaData.scriptClass->getField("EnableLighting");
		sEnableShadows = metaData.scriptClass->getField("EnableShadows");
		sOverlayOnly = metaData.scriptClass->getField("OverlayOnly");
	}

	SPtr<RenderSettings> ScriptRenderSettings::toNative(MonoObject* object)
	{
		SPtr<RenderSettings> output = bs_shared_ptr_new<RenderSettings>();

		sEnableAutoExposure->get(object, &output->enableAutoExposure);
		sEnableTonemapping->get(object, &output->enableTonemapping);
		sExposureScale->get(object, &output->exposureScale);
		sGamma->get(object, &output->gamma);
		sEnableHDR->get(object, &output->enableHDR);
		sEnableLighting->get(object, &output->enableLighting);
		sEnableShadows->get(object, &output->enableShadows);
		sOverlayOnly->get(object, &output->overlayOnly);

		MonoObject* autoExposureMono;
		sAutoExposure->get(object, &autoExposureMono);
		output->autoExposure = *ScriptAutoExposureSettings::toNative(autoExposureMono);

		MonoObject* tonemappingMono;
		sTonemapping->get(object, &tonemappingMono);
		output->tonemapping = *ScriptTonemappingSettings::toNative(tonemappingMono);

		MonoObject* whiteBalanceMono;
		sWhiteBalance->get(object, &whiteBalanceMono);
		output->whiteBalance = *ScriptWhiteBalanceSettings::toNative(whiteBalanceMono);

		MonoObject* colorGradingMono;
		sColorGrading->get(object, &colorGradingMono);
		output->colorGrading = *ScriptColorGradingSettings::toNative(colorGradingMono);

		return output;
	}

	MonoObject* ScriptRenderSettings::toManaged(const SPtr<RenderSettings>& value)
	{
		MonoObject* object = metaData.scriptClass->createInstance();

		sEnableAutoExposure->set(object, &value->enableAutoExposure);
		sEnableTonemapping->set(object, &value->enableTonemapping);
		sExposureScale->set(object, &value->exposureScale);
		sGamma->set(object, &value->gamma);
		sEnableHDR->set(object, &value->enableHDR);
		sEnableLighting->set(object, &value->enableLighting);
		sEnableShadows->set(object, &value->enableShadows);
		sOverlayOnly->set(object, &value->overlayOnly);

		MonoObject* autoExposureMono = ScriptAutoExposureSettings::toManaged(value->autoExposure);
		sAutoExposure->set(object, autoExposureMono);

		MonoObject* tonemappingMono = ScriptTonemappingSettings::toManaged(value->tonemapping);
		sTonemapping->set(object, tonemappingMono);

		MonoObject* whiteBalanceMono = ScriptWhiteBalanceSettings::toManaged(value->whiteBalance);
		sWhiteBalance->set(object, whiteBalanceMono);

		MonoObject* colorGradingMono = ScriptColorGradingSettings::toManaged(value->colorGrading);
		sColorGrading->set(object, colorGradingMono);

		return object;
	}

	MonoObject* ScriptRenderSettings::internal_CreateDefault()
	{
		return toManaged(bs_shared_ptr_new<RenderSettings>());
	}
}