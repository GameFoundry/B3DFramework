//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Renderer/BsRenderSettings.h"
#include "Private/RTTI/BsRenderSettingsRTTI.h"
#include "CoreThread/BsCoreObjectSync.h"

namespace bs
{
	template <class Processor>
	void AutoExposureSettings::rttiProcess(Processor p)
	{
		p << histogramLog2Min;
		p << histogramLog2Max;
		p << histogramPctLow;
		p << histogramPctHigh;
		p << minEyeAdaptation;
		p << maxEyeAdaptation;
		p << eyeAdaptationSpeedUp;
		p << eyeAdaptationSpeedDown;
	}

	RTTITypeBase* AutoExposureSettings::getRTTIStatic()
	{
		return AutoExposureSettingsRTTI::instance();
	}

	RTTITypeBase* AutoExposureSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void TonemappingSettings::rttiProcess(Processor p)
	{
		p << filmicCurveShoulderStrength;
		p << filmicCurveLinearStrength;
		p << filmicCurveLinearAngle;
		p << filmicCurveToeStrength;
		p << filmicCurveToeNumerator;
		p << filmicCurveToeDenominator;
		p << filmicCurveLinearWhitePoint;
	}

	RTTITypeBase* TonemappingSettings::getRTTIStatic()
	{
		return TonemappingSettingsRTTI::instance();
	}

	RTTITypeBase* TonemappingSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void WhiteBalanceSettings::rttiProcess(Processor p)
	{
		p << temperature;
		p << tint;
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTIStatic()
	{
		return WhiteBalanceSettingsRTTI::instance();
	}

	RTTITypeBase* WhiteBalanceSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void ColorGradingSettings::rttiProcess(Processor p)
	{
		p << saturation;
		p << gain;
		p << contrast;
		p << offset;
	}

	RTTITypeBase* ColorGradingSettings::getRTTIStatic()
	{
		return ColorGradingSettingsRTTI::instance();
	}

	RTTITypeBase* ColorGradingSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void AmbientOcclusionSettings::rttiProcess(Processor p)
	{
		p << enabled;
		p << radius;
		p << bias;
		p << fadeDistance;
		p << fadeRange;
		p << intensity;
		p << power;
		p << quality;
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTIStatic()
	{
		return AmbientOcclusionSettingsRTTI::instance();
	}

	RTTITypeBase* AmbientOcclusionSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void DepthOfFieldSettings::rttiProcess(Processor p)
	{
		p << enabled;
		p << focalDistance;
		p << focalRange;
		p << nearTransitionRange;
		p << farTransitionRange;
		p << nearBlurAmount;
		p << farBlurAmount;
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTIStatic()
	{
		return DepthOfFieldSettingsRTTI::instance();
	}

	RTTITypeBase* DepthOfFieldSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void ScreenSpaceReflectionsSettings::rttiProcess(Processor p)
	{
		p << enabled;
		p << quality;
		p << intensity;
		p << maxRoughness;
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTIStatic()
	{
		return ScreenSpaceReflectionsSettingsRTTI::instance();
	}

	RTTITypeBase* ScreenSpaceReflectionsSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void BloomSettings::rttiProcess(Processor p)
	{
		p << enabled;
		p << quality;
		p << threshold;
		p << intensity;
		p << tint;
	}

	RTTITypeBase* BloomSettings::getRTTIStatic()
	{
		return BloomSettingsRTTI::instance();
	}

	RTTITypeBase* BloomSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void ShadowSettings::rttiProcess(Processor p)
	{
		p << directionalShadowDistance;
		p << numCascades;
		p << cascadeDistributionExponent;
		p << shadowFilteringQuality;
	}

	RTTITypeBase* ShadowSettings::getRTTIStatic()
	{
		return ShadowSettingsRTTI::instance();
	}

	RTTITypeBase* ShadowSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	template <class Processor>
	void RenderSettings::rttiProcess(Processor p)
	{
		p << enableAutoExposure;
		p << enableTonemapping;
		p << exposureScale;
		p << gamma;
		p << enableFXAA;
		p << enableHDR;
		p << enableLighting;
		p << enableShadows;
		p << enableIndirectLighting;
		p << overlayOnly;
		p << enableSkybox;
	}

	RTTITypeBase* RenderSettings::getRTTIStatic()
	{
		return RenderSettingsRTTI::instance();
	}

	RTTITypeBase* RenderSettings::getRTTI() const
	{
		return getRTTIStatic();
	}

	void RenderSettings::_getSyncData(UINT8* buffer, UINT32& size)
	{
		UINT32 bufferSize = 0;
		const RttiCoreSyncSize sizer(bufferSize);

		rttiProcess(sizer);
		autoExposure.rttiProcess(sizer);
		tonemapping.rttiProcess(sizer);
		whiteBalance.rttiProcess(sizer);
		colorGrading.rttiProcess(sizer);
		depthOfField.rttiProcess(sizer);
		ambientOcclusion.rttiProcess(sizer);
		screenSpaceReflections.rttiProcess(sizer);
		bloom.rttiProcess(sizer);
		shadowSettings.rttiProcess(sizer);
		
		if (buffer == nullptr)
		{
			size = bufferSize;
			return;
		}

		if (bufferSize != size)
		{
			LOGERR("Invalid buffer size");
			return;
		}

		const RttiCoreSyncWriter writer((char**)&buffer);

		rttiProcess(writer);
		autoExposure.rttiProcess(writer);
		tonemapping.rttiProcess(writer);
		whiteBalance.rttiProcess(writer);
		colorGrading.rttiProcess(writer);
		depthOfField.rttiProcess(writer);
		ambientOcclusion.rttiProcess(writer);
		screenSpaceReflections.rttiProcess(writer);
		bloom.rttiProcess(writer);
		shadowSettings.rttiProcess(writer);
	}

	void RenderSettings::_setSyncData(UINT8* buffer, UINT32 size)
	{
		const RttiCoreSyncReader reader((char**)&buffer);

		rttiProcess(reader);
		autoExposure.rttiProcess(reader);
		tonemapping.rttiProcess(reader);
		whiteBalance.rttiProcess(reader);
		colorGrading.rttiProcess(reader);
		depthOfField.rttiProcess(reader);
		ambientOcclusion.rttiProcess(reader);
		screenSpaceReflections.rttiProcess(reader);
		bloom.rttiProcess(reader);
		shadowSettings.rttiProcess(reader);
	}
}
