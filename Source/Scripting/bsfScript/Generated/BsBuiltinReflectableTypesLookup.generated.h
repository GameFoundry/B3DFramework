//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Serialization/BsBuiltinReflectableTypesLookup.h"
#include "Reflection/BsRTTIType.h"
#include "../../../Foundation/bsfCore/Animation/BsSkeleton.h"
#include "BsScriptSkeleton.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptMotionBlurSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptAmbientOcclusionSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleEvolver.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTexture.h"
#include "BsScriptRenderTexture.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Importer/BsImportOptions.h"
#endif
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterStaticMeshShape.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptBloomSettings.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"
#include "BsScriptMorphShapes.generated.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptFontBitmap.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"
#include "BsScriptMorphShape.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"
#include "BsScriptMorphChannel.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptTemporalAASettings.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Audio/BsAudioClipImportOptions.h"
#endif
#include "BsScriptAudioClipImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "BsScriptPixelData.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsViewport.h"
#include "BsScriptViewport.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTarget.h"
#include "BsScriptRenderTarget.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleGravity.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptScreenSpaceLensFlareSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptAutoExposureSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptTonemappingSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptWhiteBalanceSettings.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Importer/BsShaderImportOptions.h"
#endif
#include "BsScriptShaderImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptColorGradingSettings.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfEngine/Resources/BsScriptCodeImportOptions.h"
#endif
#include "BsScriptScriptCodeImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptDepthOfFieldSettings.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#endif
#include "BsScriptImportedAnimationEvents.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptScreenSpaceReflectionsSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptChromaticAberrationSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptFilmGrainSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptShadowSettings.generated.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "BsScriptRenderSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleForce.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterCircleShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleVelocity.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleTextureAnimation.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleOrbit.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleColor.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleSize.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleRotation.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEvolver.h"
#include "BsScriptParticleCollisions.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterConeShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterSphereShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterHemisphereShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterBoxShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterLineShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterRectShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitterSkinnedMeshShape.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleEmitter.h"
#include "BsScriptParticleEmitter.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleDepthCollisionSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleVectorFieldSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleSystemSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleGpuSimulationSettings.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#endif
#include "BsScriptAnimationSplitInfo.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#endif
#include "BsScriptMeshImportOptions.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Resources/BsResourceManifest.h"
#endif
#include "BsScriptResourceManifest.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Importer/BsTextureImportOptions.h"
#endif
#include "BsScriptTextureImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"
#include "BsScriptShaderVariation.generated.h"
#if !BS_IS_BANSHEE3D
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#endif
#include "BsScriptFontImportOptions.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"
#include "BsScriptGUIElementStyle.generated.h"

namespace bs
{
	LOOKUP_BEGIN(BuiltinReflectableTypes)
		ADD_ENTRY(Skeleton, ScriptSkeleton)
		ADD_ENTRY(MotionBlurSettings, ScriptMotionBlurSettings)
		ADD_ENTRY(AmbientOcclusionSettings, ScriptAmbientOcclusionSettings)
		ADD_ENTRY(ParticleEvolver, ScriptParticleEvolver)
		ADD_ENTRY(RenderTexture, ScriptRenderTexture)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(ImportOptions, ScriptImportOptions)
#endif
		ADD_ENTRY(ParticleEmitterStaticMeshShape, ScriptParticleEmitterStaticMeshShape)
		ADD_ENTRY(BloomSettings, ScriptBloomSettings)
		ADD_ENTRY(MorphShapes, ScriptMorphShapes)
		ADD_ENTRY(FontBitmap, ScriptFontBitmap)
		ADD_ENTRY(MorphShape, ScriptMorphShape)
		ADD_ENTRY(MorphChannel, ScriptMorphChannel)
		ADD_ENTRY(TemporalAASettings, ScriptTemporalAASettings)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(AudioClipImportOptions, ScriptAudioClipImportOptions)
#endif
		ADD_ENTRY(PixelData, ScriptPixelData)
		ADD_ENTRY(Viewport, ScriptViewport)
		ADD_ENTRY(RenderTarget, ScriptRenderTarget)
		ADD_ENTRY(ParticleGravity, ScriptParticleGravity)
		ADD_ENTRY(ScreenSpaceLensFlareSettings, ScriptScreenSpaceLensFlareSettings)
		ADD_ENTRY(AutoExposureSettings, ScriptAutoExposureSettings)
		ADD_ENTRY(TonemappingSettings, ScriptTonemappingSettings)
		ADD_ENTRY(WhiteBalanceSettings, ScriptWhiteBalanceSettings)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(ShaderImportOptions, ScriptShaderImportOptions)
#endif
		ADD_ENTRY(ColorGradingSettings, ScriptColorGradingSettings)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(ScriptCodeImportOptions, ScriptScriptCodeImportOptions)
#endif
		ADD_ENTRY(DepthOfFieldSettings, ScriptDepthOfFieldSettings)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(ImportedAnimationEvents, ScriptImportedAnimationEvents)
#endif
		ADD_ENTRY(ScreenSpaceReflectionsSettings, ScriptScreenSpaceReflectionsSettings)
		ADD_ENTRY(ChromaticAberrationSettings, ScriptChromaticAberrationSettings)
		ADD_ENTRY(FilmGrainSettings, ScriptFilmGrainSettings)
		ADD_ENTRY(ShadowSettings, ScriptShadowSettings)
		ADD_ENTRY(RenderSettings, ScriptRenderSettings)
		ADD_ENTRY(ParticleForce, ScriptParticleForce)
		ADD_ENTRY(ParticleEmitterCircleShape, ScriptParticleEmitterCircleShape)
		ADD_ENTRY(ParticleVelocity, ScriptParticleVelocity)
		ADD_ENTRY(ParticleTextureAnimation, ScriptParticleTextureAnimation)
		ADD_ENTRY(ParticleOrbit, ScriptParticleOrbit)
		ADD_ENTRY(ParticleColor, ScriptParticleColor)
		ADD_ENTRY(ParticleSize, ScriptParticleSize)
		ADD_ENTRY(ParticleRotation, ScriptParticleRotation)
		ADD_ENTRY(ParticleCollisions, ScriptParticleCollisions)
		ADD_ENTRY(ParticleEmitterShape, ScriptParticleEmitterShape)
		ADD_ENTRY(ParticleEmitterConeShape, ScriptParticleEmitterConeShape)
		ADD_ENTRY(ParticleEmitterSphereShape, ScriptParticleEmitterSphereShape)
		ADD_ENTRY(ParticleEmitterHemisphereShape, ScriptParticleEmitterHemisphereShape)
		ADD_ENTRY(ParticleEmitterBoxShape, ScriptParticleEmitterBoxShape)
		ADD_ENTRY(ParticleEmitterLineShape, ScriptParticleEmitterLineShape)
		ADD_ENTRY(ParticleEmitterRectShape, ScriptParticleEmitterRectShape)
		ADD_ENTRY(ParticleEmitterSkinnedMeshShape, ScriptParticleEmitterSkinnedMeshShape)
		ADD_ENTRY(ParticleEmitter, ScriptParticleEmitter)
		ADD_ENTRY(ParticleDepthCollisionSettings, ScriptParticleDepthCollisionSettings)
		ADD_ENTRY(ParticleVectorFieldSettings, ScriptParticleVectorFieldSettings)
		ADD_ENTRY(ParticleSystemSettings, ScriptParticleSystemSettings)
		ADD_ENTRY(ParticleGpuSimulationSettings, ScriptParticleGpuSimulationSettings)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(AnimationSplitInfo, ScriptAnimationSplitInfo)
#endif
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(MeshImportOptions, ScriptMeshImportOptions)
#endif
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(ResourceManifest, ScriptResourceManifest)
#endif
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(TextureImportOptions, ScriptTextureImportOptions)
#endif
		ADD_ENTRY(ShaderVariation, ScriptShaderVariation)
#if !BS_IS_BANSHEE3D
		ADD_ENTRY(FontImportOptions, ScriptFontImportOptions)
#endif
		ADD_ENTRY(GUIElementStyle, ScriptGUIElementStyle)
	LOOKUP_END
}
#undef LOOKUP_BEGIN
#undef ADD_ENTRY
#undef LOOKUP_END
