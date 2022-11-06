//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"
#include "../../../Foundation/bsfCore/Renderer/BsRenderSettings.h"

namespace bs { struct RenderSettings; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderSettings : public TScriptReflectable<ScriptRenderSettings, RenderSettings>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "RenderSettings")

		ScriptRenderSettings(MonoObject* managedInstance, const SPtr<RenderSettings>& value);

		static MonoObject* Create(const SPtr<RenderSettings>& value);

	private:
		static void InternalRenderSettings(MonoObject* managedInstance);
		static MonoObject* InternalGetDepthOfField(ScriptRenderSettings* thisPtr);
		static void InternalSetDepthOfField(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetChromaticAberration(ScriptRenderSettings* thisPtr);
		static void InternalSetChromaticAberration(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool InternalGetEnableAutoExposure(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableAutoExposure(ScriptRenderSettings* thisPtr, bool value);
		static MonoObject* InternalGetAutoExposure(ScriptRenderSettings* thisPtr);
		static void InternalSetAutoExposure(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool InternalGetEnableTonemapping(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableTonemapping(ScriptRenderSettings* thisPtr, bool value);
		static MonoObject* InternalGetTonemapping(ScriptRenderSettings* thisPtr);
		static void InternalSetTonemapping(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetWhiteBalance(ScriptRenderSettings* thisPtr);
		static void InternalSetWhiteBalance(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetColorGrading(ScriptRenderSettings* thisPtr);
		static void InternalSetColorGrading(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetAmbientOcclusion(ScriptRenderSettings* thisPtr);
		static void InternalSetAmbientOcclusion(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetScreenSpaceReflections(ScriptRenderSettings* thisPtr);
		static void InternalSetScreenSpaceReflections(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetBloom(ScriptRenderSettings* thisPtr);
		static void InternalSetBloom(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetScreenSpaceLensFlare(ScriptRenderSettings* thisPtr);
		static void InternalSetScreenSpaceLensFlare(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetFilmGrain(ScriptRenderSettings* thisPtr);
		static void InternalSetFilmGrain(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetMotionBlur(ScriptRenderSettings* thisPtr);
		static void InternalSetMotionBlur(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* InternalGetTemporalAa(ScriptRenderSettings* thisPtr);
		static void InternalSetTemporalAa(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool InternalGetEnableFxaa(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableFxaa(ScriptRenderSettings* thisPtr, bool value);
		static float InternalGetExposureScale(ScriptRenderSettings* thisPtr);
		static void InternalSetExposureScale(ScriptRenderSettings* thisPtr, float value);
		static float InternalGetGamma(ScriptRenderSettings* thisPtr);
		static void InternalSetGamma(ScriptRenderSettings* thisPtr, float value);
		static bool InternalGetEnableHdr(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableHdr(ScriptRenderSettings* thisPtr, bool value);
		static bool InternalGetEnableLighting(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableLighting(ScriptRenderSettings* thisPtr, bool value);
		static bool InternalGetEnableShadows(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableShadows(ScriptRenderSettings* thisPtr, bool value);
		static bool InternalGetEnableVelocityBuffer(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableVelocityBuffer(ScriptRenderSettings* thisPtr, bool value);
		static MonoObject* InternalGetShadowSettings(ScriptRenderSettings* thisPtr);
		static void InternalSetShadowSettings(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool InternalGetEnableIndirectLighting(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableIndirectLighting(ScriptRenderSettings* thisPtr, bool value);
		static bool InternalGetOverlayOnly(ScriptRenderSettings* thisPtr);
		static void InternalSetOverlayOnly(ScriptRenderSettings* thisPtr, bool value);
		static bool InternalGetEnableSkybox(ScriptRenderSettings* thisPtr);
		static void InternalSetEnableSkybox(ScriptRenderSettings* thisPtr, bool value);
		static float InternalGetCullDistance(ScriptRenderSettings* thisPtr);
		static void InternalSetCullDistance(ScriptRenderSettings* thisPtr, float value);
	};
}
