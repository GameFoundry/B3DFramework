//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DColorRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "Renderer/B3DRenderSettings.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT AutoExposureSettingsRTTI : public TRTTIType<AutoExposureSettings, IReflectable, AutoExposureSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(HistogramLog2Min, 0)
			B3D_RTTI_MEMBER(HistogramLog2Max, 1)
			B3D_RTTI_MEMBER(HistogramPctLow, 2)
			B3D_RTTI_MEMBER(HistogramPctHigh, 3)
			B3D_RTTI_MEMBER(MinEyeAdaptation, 4)
			B3D_RTTI_MEMBER(MaxEyeAdaptation, 5)
			B3D_RTTI_MEMBER(EyeAdaptationSpeedUp, 6)
			B3D_RTTI_MEMBER(EyeAdaptationSpeedDown, 7)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName()
		{
			static String name = "AutoExposureSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AutoExposureSettings;
		}

		SPtr<IReflectable> NewRttiObject()
		{
			return B3DMakeShared<AutoExposureSettings>();
		}
	};

	class B3D_EXPORT TonemappingSettingsRTTI : public TRTTIType<TonemappingSettings, IReflectable, TonemappingSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(FilmicCurveShoulderStrength, 0)
			B3D_RTTI_MEMBER(FilmicCurveLinearStrength, 1)
			B3D_RTTI_MEMBER(FilmicCurveLinearAngle, 2)
			B3D_RTTI_MEMBER(FilmicCurveToeStrength, 3)
			B3D_RTTI_MEMBER(FilmicCurveToeNumerator, 4)
			B3D_RTTI_MEMBER(FilmicCurveToeDenominator, 5)
			B3D_RTTI_MEMBER(FilmicCurveLinearWhitePoint, 6)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "TonemappingSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_TonemappingSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<TonemappingSettings>();
		}
	};

	class B3D_EXPORT WhiteBalanceSettingsRTTI : public TRTTIType<WhiteBalanceSettings, IReflectable, WhiteBalanceSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Temperature, 0)
			B3D_RTTI_MEMBER(Tint, 1)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "WhiteBalanceSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_WhiteBalanceSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<WhiteBalanceSettings>();
		}
	};

	class B3D_EXPORT ColorGradingSettingsRTTI : public TRTTIType<ColorGradingSettings, IReflectable, ColorGradingSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Saturation, 0)
			B3D_RTTI_MEMBER(Contrast, 1)
			B3D_RTTI_MEMBER(Gain, 2)
			B3D_RTTI_MEMBER(Offset, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ColorGradingSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ColorGradingSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ColorGradingSettings>();
		}
	};

	class B3D_EXPORT DepthOfFieldSettingsRTTI : public TRTTIType<DepthOfFieldSettings, IReflectable, DepthOfFieldSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(FocalDistance, 1)
			B3D_RTTI_MEMBER(FocalRange, 2)
			B3D_RTTI_MEMBER(NearTransitionRange, 3)
			B3D_RTTI_MEMBER(FarTransitionRange, 4)
			B3D_RTTI_MEMBER(NearBlurAmount, 5)
			B3D_RTTI_MEMBER(FarBlurAmount, 6)
			B3D_RTTI_MEMBER(Type, 7)
			B3D_RTTI_MEMBER(MaxBokehSize, 8)
			B3D_RTTI_MEMBER(BokehShape, 9)
			B3D_RTTI_MEMBER(AdaptiveColorThreshold, 10)
			B3D_RTTI_MEMBER(AdaptiveRadiusThreshold, 11)
			B3D_RTTI_MEMBER(ApertureSize, 12)
			B3D_RTTI_MEMBER(FocalLength, 13)
			B3D_RTTI_MEMBER(SensorSize, 14)
			B3D_RTTI_MEMBER(BokehOcclusion, 15)
			B3D_RTTI_MEMBER(OcclusionDepthRange, 16)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "DepthOfFieldSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_DepthOfFieldSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<DepthOfFieldSettings>();
		}
	};

	class B3D_EXPORT AmbientOcclusionSettingsRTTI : public TRTTIType<AmbientOcclusionSettings, IReflectable, AmbientOcclusionSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Radius, 1)
			B3D_RTTI_MEMBER(Bias, 2)
			B3D_RTTI_MEMBER(FadeRange, 3)
			B3D_RTTI_MEMBER(FadeDistance, 4)
			B3D_RTTI_MEMBER(Intensity, 5)
			B3D_RTTI_MEMBER(Power, 6)
			B3D_RTTI_MEMBER(Quality, 7)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "AmbientOcclusionSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_AmbientOcclusionSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<AmbientOcclusionSettings>();
		}
	};

	class B3D_EXPORT MotionBlurSettingsRTTI : public TRTTIType<MotionBlurSettings, IReflectable, MotionBlurSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Domain, 1)
			B3D_RTTI_MEMBER(Filter, 2)
			B3D_RTTI_MEMBER(Quality, 3)
			B3D_RTTI_MEMBER(MaximumRadius, 4)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "MotionBlurSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MotionBlurSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<MotionBlurSettings>();
		}
	};

	class B3D_EXPORT TemporalAASettingsRTTI : public TRTTIType<TemporalAASettings, IReflectable, TemporalAASettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(JitteredPositionCount, 1)
			B3D_RTTI_MEMBER(Sharpness, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "TemporalAASettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_TemporalAASettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<TemporalAASettings>();
		}
	};

	class B3D_EXPORT ScreenSpaceReflectionsSettingsRTTI : public TRTTIType<ScreenSpaceReflectionsSettings, IReflectable, ScreenSpaceReflectionsSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Intensity, 1)
			B3D_RTTI_MEMBER(MaxRoughness, 2)
			B3D_RTTI_MEMBER(Quality, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ScreenSpaceReflectionsSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScreenSpaceReflectionsSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ScreenSpaceReflectionsSettings>();
		}
	};

	class B3D_EXPORT BloomSettingsRTTI : public TRTTIType<BloomSettings, IReflectable, BloomSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Quality, 1)
			B3D_RTTI_MEMBER(Threshold, 2)
			B3D_RTTI_MEMBER(Intensity, 3)
			B3D_RTTI_MEMBER(Tint, 4)
			B3D_RTTI_MEMBER(FilterSize, 5)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "BloomSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_BloomSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<BloomSettings>();
		}
	};

	class B3D_EXPORT ScreenSpaceLensFlareSettingsRTTI : public TRTTIType<ScreenSpaceLensFlareSettings, IReflectable, ScreenSpaceLensFlareSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(DownsampleCount, 1)
			B3D_RTTI_MEMBER(Threshold, 2)
			B3D_RTTI_MEMBER(GhostCount, 3)
			B3D_RTTI_MEMBER(GhostSpacing, 4)
			B3D_RTTI_MEMBER(Brightness, 5)
			B3D_RTTI_MEMBER(FilterSize, 6)
			B3D_RTTI_MEMBER(Halo, 7)
			B3D_RTTI_MEMBER(HaloAspectRatio, 8)
			B3D_RTTI_MEMBER(HaloRadius, 9)
			B3D_RTTI_MEMBER(HaloThickness, 10)
			B3D_RTTI_MEMBER(HaloThreshold, 11)
			B3D_RTTI_MEMBER(ChromaticAberration, 12)
			B3D_RTTI_MEMBER(ChromaticAberrationOffset, 13)
			B3D_RTTI_MEMBER(BicubicUpsampling, 14)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ScreenSpaceLensFlareSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ScreenSpaceLensFlareSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ScreenSpaceLensFlareSettings>();
		}
	};

	class B3D_EXPORT ChromaticAberrationSettingsRTTI : public TRTTIType<ChromaticAberrationSettings, IReflectable, ChromaticAberrationSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Type, 1)
			B3D_RTTI_MEMBER(ShiftAmount, 2)
			B3D_RTTI_MEMBER(FringeTexture, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ChromaticAberrationSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ChromaticAberrationSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ChromaticAberrationSettings>();
		}
	};

	class B3D_EXPORT FilmGrainSettingsRTTI : public TRTTIType<FilmGrainSettings, IReflectable, FilmGrainSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Enabled, 0)
			B3D_RTTI_MEMBER(Intensity, 1)
			B3D_RTTI_MEMBER(Speed, 2)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "FilmGrainSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_FilmGrainSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<FilmGrainSettings>();
		}
	};

	class B3D_EXPORT ShadowSettingsRTTI : public TRTTIType<ShadowSettings, IReflectable, ShadowSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(DirectionalShadowDistance, 0)
			B3D_RTTI_MEMBER(NumCascades, 1)
			B3D_RTTI_MEMBER(CascadeDistributionExponent, 2)
			B3D_RTTI_MEMBER(ShadowFilteringQuality, 3)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "ShadowSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ShadowSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ShadowSettings>();
		}
	};

	class B3D_EXPORT RenderSettingsRTTI : public TRTTIType<RenderSettings, IReflectable, RenderSettingsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(EnableAutoExposure, 0)
			B3D_RTTI_MEMBER(AutoExposure, 1)
			B3D_RTTI_MEMBER(EnableTonemapping, 2)
			B3D_RTTI_MEMBER(Tonemapping, 3)
			B3D_RTTI_MEMBER(WhiteBalance, 4)
			B3D_RTTI_MEMBER(ColorGrading, 5)
			B3D_RTTI_MEMBER(ExposureScale, 6)
			B3D_RTTI_MEMBER(Gamma, 7)
			B3D_RTTI_MEMBER(DepthOfField, 8)
			B3D_RTTI_MEMBER(EnableFxaa, 9)
			B3D_RTTI_MEMBER(AmbientOcclusion, 10)
			B3D_RTTI_MEMBER(ScreenSpaceReflections, 11)
			B3D_RTTI_MEMBER(EnableHdr, 12)
			B3D_RTTI_MEMBER(EnableLighting, 13)
			B3D_RTTI_MEMBER(EnableShadows, 14)
			B3D_RTTI_MEMBER(OverlayOnly, 15)
			B3D_RTTI_MEMBER(EnableIndirectLighting, 16)
			B3D_RTTI_MEMBER(ShadowSettings, 17)
			B3D_RTTI_MEMBER(EnableSkybox, 18)
			B3D_RTTI_MEMBER(Bloom, 19)
			B3D_RTTI_MEMBER(ScreenSpaceLensFlare, 20)
			B3D_RTTI_MEMBER(MotionBlur, 21)
			B3D_RTTI_MEMBER(FilmGrain, 22)
			B3D_RTTI_MEMBER(ChromaticAberration, 23)
			B3D_RTTI_MEMBER(TemporalAa, 24)
			B3D_RTTI_MEMBER(EnableVelocityBuffer, 25)
		B3D_RTTI_END_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "RenderSettings";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_RenderSettings;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<RenderSettings>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace b3d
