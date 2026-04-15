//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

namespace b3d
{
	/** @addtogroup Rendering-Internal
	 *  @{
	 */

	B3D_SYNC_BLOCK_BEGIN(AutoExposureSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(HistogramLog2Min)
		B3D_SYNC_BLOCK_ENTRY(HistogramLog2Max)
		B3D_SYNC_BLOCK_ENTRY(HistogramPctLow)
		B3D_SYNC_BLOCK_ENTRY(HistogramPctHigh)
		B3D_SYNC_BLOCK_ENTRY(MinEyeAdaptation)
		B3D_SYNC_BLOCK_ENTRY(MaxEyeAdaptation)
		B3D_SYNC_BLOCK_ENTRY(EyeAdaptationSpeedUp)
		B3D_SYNC_BLOCK_ENTRY(EyeAdaptationSpeedDown)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(TonemappingSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveShoulderStrength)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveLinearStrength)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveLinearAngle)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveToeStrength)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveToeNumerator)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveToeDenominator)
		B3D_SYNC_BLOCK_ENTRY(FilmicCurveLinearWhitePoint)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(WhiteBalanceSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Temperature)
		B3D_SYNC_BLOCK_ENTRY(Tint)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(ColorGradingSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Saturation)
		B3D_SYNC_BLOCK_ENTRY(Gain)
		B3D_SYNC_BLOCK_ENTRY(Contrast)
		B3D_SYNC_BLOCK_ENTRY(Offset)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(AmbientOcclusionSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Radius)
		B3D_SYNC_BLOCK_ENTRY(Bias)
		B3D_SYNC_BLOCK_ENTRY(FadeDistance)
		B3D_SYNC_BLOCK_ENTRY(FadeRange)
		B3D_SYNC_BLOCK_ENTRY(Intensity)
		B3D_SYNC_BLOCK_ENTRY(Power)
		B3D_SYNC_BLOCK_ENTRY(Quality)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(DepthOfFieldSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(FocalDistance)
		B3D_SYNC_BLOCK_ENTRY(FocalRange)
		B3D_SYNC_BLOCK_ENTRY(NearTransitionRange)
		B3D_SYNC_BLOCK_ENTRY(FarTransitionRange)
		B3D_SYNC_BLOCK_ENTRY(NearBlurAmount)
		B3D_SYNC_BLOCK_ENTRY(FarBlurAmount)
		B3D_SYNC_BLOCK_ENTRY(Type)
		B3D_SYNC_BLOCK_ENTRY(MaxBokehSize)
		B3D_SYNC_BLOCK_ENTRY(BokehShape)
		B3D_SYNC_BLOCK_ENTRY(AdaptiveColorThreshold)
		B3D_SYNC_BLOCK_ENTRY(AdaptiveRadiusThreshold)
		B3D_SYNC_BLOCK_ENTRY(FocalLength)
		B3D_SYNC_BLOCK_ENTRY(ApertureSize)
		B3D_SYNC_BLOCK_ENTRY(SensorSize)
		B3D_SYNC_BLOCK_ENTRY(BokehOcclusion)
		B3D_SYNC_BLOCK_ENTRY(OcclusionDepthRange)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(ScreenSpaceReflectionsSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Quality)
		B3D_SYNC_BLOCK_ENTRY(Intensity)
		B3D_SYNC_BLOCK_ENTRY(MaxRoughness)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(BloomSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Quality)
		B3D_SYNC_BLOCK_ENTRY(Threshold)
		B3D_SYNC_BLOCK_ENTRY(Intensity)
		B3D_SYNC_BLOCK_ENTRY(Tint)
		B3D_SYNC_BLOCK_ENTRY(FilterSize)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(ScreenSpaceLensFlareSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(DownsampleCount)
		B3D_SYNC_BLOCK_ENTRY(Threshold)
		B3D_SYNC_BLOCK_ENTRY(GhostCount)
		B3D_SYNC_BLOCK_ENTRY(GhostSpacing)
		B3D_SYNC_BLOCK_ENTRY(Brightness)
		B3D_SYNC_BLOCK_ENTRY(FilterSize)
		B3D_SYNC_BLOCK_ENTRY(Halo)
		B3D_SYNC_BLOCK_ENTRY(HaloRadius)
		B3D_SYNC_BLOCK_ENTRY(HaloThickness)
		B3D_SYNC_BLOCK_ENTRY(HaloThreshold)
		B3D_SYNC_BLOCK_ENTRY(HaloAspectRatio)
		B3D_SYNC_BLOCK_ENTRY(ChromaticAberration)
		B3D_SYNC_BLOCK_ENTRY(ChromaticAberrationOffset)
		B3D_SYNC_BLOCK_ENTRY(BicubicUpsampling)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(MotionBlurSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Domain)
		B3D_SYNC_BLOCK_ENTRY(Filter)
		B3D_SYNC_BLOCK_ENTRY(Quality)
		B3D_SYNC_BLOCK_ENTRY(MaximumRadius)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(TemporalAASettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(JitteredPositionCount)
		B3D_SYNC_BLOCK_ENTRY(Sharpness)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(ChromaticAberrationSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Type)
		B3D_SYNC_BLOCK_ENTRY(ShiftAmount)
		B3D_SYNC_BLOCK_ENTRY(FringeTexture)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(FilmGrainSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(Enabled)
		B3D_SYNC_BLOCK_ENTRY(Intensity)
		B3D_SYNC_BLOCK_ENTRY(Speed)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(ShadowSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(DirectionalShadowDistance)
		B3D_SYNC_BLOCK_ENTRY(NumCascades)
		B3D_SYNC_BLOCK_ENTRY(CascadeDistributionExponent)
		B3D_SYNC_BLOCK_ENTRY(ShadowFilteringQuality)
	B3D_SYNC_BLOCK_END

	B3D_SYNC_BLOCK_BEGIN(RenderSettings, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(EnableAutoExposure)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(AutoExposure, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(EnableTonemapping)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(Tonemapping, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(WhiteBalance, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(ColorGrading, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(DepthOfField, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(AmbientOcclusion, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(ScreenSpaceReflections, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(Bloom, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(ScreenSpaceLensFlare, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(ExposureScale)
		B3D_SYNC_BLOCK_ENTRY(Gamma)
		B3D_SYNC_BLOCK_ENTRY(EnableFxaa)
		B3D_SYNC_BLOCK_ENTRY(EnableHdr)
		B3D_SYNC_BLOCK_ENTRY(EnableLighting)
		B3D_SYNC_BLOCK_ENTRY(EnableShadows)
		B3D_SYNC_BLOCK_ENTRY(EnableIndirectLighting)
		B3D_SYNC_BLOCK_ENTRY(OverlayOnly)
		B3D_SYNC_BLOCK_ENTRY(EnableSkybox)
		B3D_SYNC_BLOCK_ENTRY(CullDistance)
		B3D_SYNC_BLOCK_ENTRY(MotionBlur)
		B3D_SYNC_BLOCK_ENTRY(FilmGrain)
		B3D_SYNC_BLOCK_ENTRY_PACKET_FIELD(ChromaticAberration, SyncPacket)
		B3D_SYNC_BLOCK_ENTRY(TemporalAa)
		B3D_SYNC_BLOCK_ENTRY(EnableVelocityBuffer)
	B3D_SYNC_BLOCK_END

	/** @} */
} // namespace b3d
