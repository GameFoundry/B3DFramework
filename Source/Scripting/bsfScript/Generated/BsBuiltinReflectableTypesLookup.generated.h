//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Serialization/BsBuiltinReflectableTypesLookup.h"
#include "Reflection/BsRTTIType.h"
#include "../../../Foundation/bsfCore/Animation/BsSkeleton.h"
#include "BsScriptSkeleton.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTexture.h"
#include "BsScriptRenderTexture.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Importer/BsImportOptions.h"
#endif
#include "BsScriptImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"
#include "BsScriptMorphShapes.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"
#include "BsScriptMorphShape.generated.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"
#include "BsScriptMorphChannel.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Audio/BsAudioClipImportOptions.h"
#endif
#include "BsScriptAudioClipImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Image/BsPixelData.h"
#include "BsScriptPixelData.generated.h"
#include "../../../Foundation/bsfCore/RenderAPI/BsRenderTarget.h"
#include "BsScriptRenderTarget.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Importer/BsShaderImportOptions.h"
#endif
#include "BsScriptShaderImportOptions.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfEngine/Resources/BsScriptCodeImportOptions.h"
#endif
#include "BsScriptScriptCodeImportOptions.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#endif
#include "BsScriptImportedAnimationEvents.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleDepthCollisionSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleVectorFieldSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleSystemSettings.generated.h"
#include "../../../Foundation/bsfCore/Particles/BsParticleSystem.h"
#include "BsScriptParticleGpuSimulationSettings.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#endif
#include "BsScriptAnimationSplitInfo.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Importer/BsMeshImportOptions.h"
#endif
#include "BsScriptMeshImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Material/BsShaderVariation.h"
#include "BsScriptShaderVariationParameters.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Importer/BsTextureImportOptions.h"
#endif
#include "BsScriptTextureImportOptions.generated.h"
#include "../../../Foundation/bsfCore/Text/BsFont.h"
#include "BsScriptFontBitmapInformation.generated.h"
#if !B3D_IS_ENGINE
#include "../../../Foundation/bsfCore/Text/BsFontImportOptions.h"
#endif
#include "BsScriptFontImportOptions.generated.h"

namespace bs
{
	LOOKUP_BEGIN(BuiltinReflectableTypes)
		ADD_ENTRY(Skeleton, ScriptSkeleton)
		ADD_ENTRY(RenderTexture, ScriptRenderTexture)
#if !B3D_IS_ENGINE
		ADD_ENTRY(ImportOptions, ScriptImportOptions)
#endif
		ADD_ENTRY(MorphShapes, ScriptMorphShapes)
		ADD_ENTRY(MorphShape, ScriptMorphShape)
		ADD_ENTRY(MorphChannel, ScriptMorphChannel)
#if !B3D_IS_ENGINE
		ADD_ENTRY(AudioClipImportOptions, ScriptAudioClipImportOptions)
#endif
		ADD_ENTRY(PixelData, ScriptPixelData)
		ADD_ENTRY(RenderTarget, ScriptRenderTarget)
#if !B3D_IS_ENGINE
		ADD_ENTRY(ShaderImportOptions, ScriptShaderImportOptions)
#endif
#if !B3D_IS_ENGINE
		ADD_ENTRY(ScriptCodeImportOptions, ScriptScriptCodeImportOptions)
#endif
#if !B3D_IS_ENGINE
		ADD_ENTRY(ImportedAnimationEvents, ScriptImportedAnimationEvents)
#endif
		ADD_ENTRY(ParticleDepthCollisionSettings, ScriptParticleDepthCollisionSettings)
		ADD_ENTRY(ParticleVectorFieldSettings, ScriptParticleVectorFieldSettings)
		ADD_ENTRY(ParticleSystemSettings, ScriptParticleSystemSettings)
		ADD_ENTRY(ParticleGpuSimulationSettings, ScriptParticleGpuSimulationSettings)
#if !B3D_IS_ENGINE
		ADD_ENTRY(AnimationSplitInfo, ScriptAnimationSplitInfo)
#endif
#if !B3D_IS_ENGINE
		ADD_ENTRY(MeshImportOptions, ScriptMeshImportOptions)
#endif
		ADD_ENTRY(ShaderVariationParameters, ScriptShaderVariationParameters)
#if !B3D_IS_ENGINE
		ADD_ENTRY(TextureImportOptions, ScriptTextureImportOptions)
#endif
		ADD_ENTRY(FontBitmapInformation, ScriptFontBitmapInformation)
#if !B3D_IS_ENGINE
		ADD_ENTRY(FontImportOptions, ScriptFontImportOptions)
#endif
	LOOKUP_END
}
#undef LOOKUP_BEGIN
#undef ADD_ENTRY
#undef LOOKUP_END
