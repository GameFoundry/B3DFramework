//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
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
	class BS_SCR_BE_EXPORT ScriptRenderSettings : public TScriptReflectable<ScriptRenderSettings, RenderSettings>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "RenderSettings")

		ScriptRenderSettings(MonoObject* managedInstance, const SPtr<RenderSettings>& value);

		static MonoObject* create(const SPtr<RenderSettings>& value);

	private:
		static void Internal_RenderSettings(MonoObject* managedInstance);
		static MonoObject* Internal_getdepthOfField(ScriptRenderSettings* thisPtr);
		static void Internal_setdepthOfField(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getchromaticAberration(ScriptRenderSettings* thisPtr);
		static void Internal_setchromaticAberration(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool Internal_getenableAutoExposure(ScriptRenderSettings* thisPtr);
		static void Internal_setenableAutoExposure(ScriptRenderSettings* thisPtr, bool value);
		static MonoObject* Internal_getautoExposure(ScriptRenderSettings* thisPtr);
		static void Internal_setautoExposure(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool Internal_getenableTonemapping(ScriptRenderSettings* thisPtr);
		static void Internal_setenableTonemapping(ScriptRenderSettings* thisPtr, bool value);
		static MonoObject* Internal_gettonemapping(ScriptRenderSettings* thisPtr);
		static void Internal_settonemapping(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getwhiteBalance(ScriptRenderSettings* thisPtr);
		static void Internal_setwhiteBalance(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getcolorGrading(ScriptRenderSettings* thisPtr);
		static void Internal_setcolorGrading(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getambientOcclusion(ScriptRenderSettings* thisPtr);
		static void Internal_setambientOcclusion(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getscreenSpaceReflections(ScriptRenderSettings* thisPtr);
		static void Internal_setscreenSpaceReflections(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getbloom(ScriptRenderSettings* thisPtr);
		static void Internal_setbloom(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getscreenSpaceLensFlare(ScriptRenderSettings* thisPtr);
		static void Internal_setscreenSpaceLensFlare(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getfilmGrain(ScriptRenderSettings* thisPtr);
		static void Internal_setfilmGrain(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_getmotionBlur(ScriptRenderSettings* thisPtr);
		static void Internal_setmotionBlur(ScriptRenderSettings* thisPtr, MonoObject* value);
		static MonoObject* Internal_gettemporalAA(ScriptRenderSettings* thisPtr);
		static void Internal_settemporalAA(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool Internal_getenableFXAA(ScriptRenderSettings* thisPtr);
		static void Internal_setenableFXAA(ScriptRenderSettings* thisPtr, bool value);
		static float Internal_getexposureScale(ScriptRenderSettings* thisPtr);
		static void Internal_setexposureScale(ScriptRenderSettings* thisPtr, float value);
		static float Internal_getgamma(ScriptRenderSettings* thisPtr);
		static void Internal_setgamma(ScriptRenderSettings* thisPtr, float value);
		static bool Internal_getenableHDR(ScriptRenderSettings* thisPtr);
		static void Internal_setenableHDR(ScriptRenderSettings* thisPtr, bool value);
		static bool Internal_getenableLighting(ScriptRenderSettings* thisPtr);
		static void Internal_setenableLighting(ScriptRenderSettings* thisPtr, bool value);
		static bool Internal_getenableShadows(ScriptRenderSettings* thisPtr);
		static void Internal_setenableShadows(ScriptRenderSettings* thisPtr, bool value);
		static bool Internal_getenableVelocityBuffer(ScriptRenderSettings* thisPtr);
		static void Internal_setenableVelocityBuffer(ScriptRenderSettings* thisPtr, bool value);
		static MonoObject* Internal_getshadowSettings(ScriptRenderSettings* thisPtr);
		static void Internal_setshadowSettings(ScriptRenderSettings* thisPtr, MonoObject* value);
		static bool Internal_getenableIndirectLighting(ScriptRenderSettings* thisPtr);
		static void Internal_setenableIndirectLighting(ScriptRenderSettings* thisPtr, bool value);
		static bool Internal_getoverlayOnly(ScriptRenderSettings* thisPtr);
		static void Internal_setoverlayOnly(ScriptRenderSettings* thisPtr, bool value);
		static bool Internal_getenableSkybox(ScriptRenderSettings* thisPtr);
		static void Internal_setenableSkybox(ScriptRenderSettings* thisPtr, bool value);
		static float Internal_getcullDistance(ScriptRenderSettings* thisPtr);
		static void Internal_setcullDistance(ScriptRenderSettings* thisPtr, float value);
	};
}
