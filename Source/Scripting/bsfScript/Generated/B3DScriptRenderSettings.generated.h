//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"
#include "../../../Engine/Core/Renderer/B3DRenderSettings.h"

namespace b3d { struct RenderSettings; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptRenderSettings : public TScriptReflectableWrapper<RenderSettings, ScriptRenderSettings>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "RenderSettings")

		ScriptRenderSettings(const TShared<RenderSettings>& nativeObject);
		~ScriptRenderSettings();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalRenderSettings(MonoObject* scriptObject);
		static MonoObject* InternalGetDepthOfField(ScriptRenderSettings* self);
		static void InternalSetDepthOfField(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetChromaticAberration(ScriptRenderSettings* self);
		static void InternalSetChromaticAberration(ScriptRenderSettings* self, MonoObject* value);
		static bool InternalGetEnableAutoExposure(ScriptRenderSettings* self);
		static void InternalSetEnableAutoExposure(ScriptRenderSettings* self, bool value);
		static MonoObject* InternalGetAutoExposure(ScriptRenderSettings* self);
		static void InternalSetAutoExposure(ScriptRenderSettings* self, MonoObject* value);
		static bool InternalGetEnableTonemapping(ScriptRenderSettings* self);
		static void InternalSetEnableTonemapping(ScriptRenderSettings* self, bool value);
		static MonoObject* InternalGetTonemapping(ScriptRenderSettings* self);
		static void InternalSetTonemapping(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetWhiteBalance(ScriptRenderSettings* self);
		static void InternalSetWhiteBalance(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetColorGrading(ScriptRenderSettings* self);
		static void InternalSetColorGrading(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetAmbientOcclusion(ScriptRenderSettings* self);
		static void InternalSetAmbientOcclusion(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetScreenSpaceReflections(ScriptRenderSettings* self);
		static void InternalSetScreenSpaceReflections(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetBloom(ScriptRenderSettings* self);
		static void InternalSetBloom(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetScreenSpaceLensFlare(ScriptRenderSettings* self);
		static void InternalSetScreenSpaceLensFlare(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetFilmGrain(ScriptRenderSettings* self);
		static void InternalSetFilmGrain(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetMotionBlur(ScriptRenderSettings* self);
		static void InternalSetMotionBlur(ScriptRenderSettings* self, MonoObject* value);
		static MonoObject* InternalGetTemporalAa(ScriptRenderSettings* self);
		static void InternalSetTemporalAa(ScriptRenderSettings* self, MonoObject* value);
		static bool InternalGetEnableFxaa(ScriptRenderSettings* self);
		static void InternalSetEnableFxaa(ScriptRenderSettings* self, bool value);
		static float InternalGetExposureScale(ScriptRenderSettings* self);
		static void InternalSetExposureScale(ScriptRenderSettings* self, float value);
		static float InternalGetGamma(ScriptRenderSettings* self);
		static void InternalSetGamma(ScriptRenderSettings* self, float value);
		static bool InternalGetEnableHdr(ScriptRenderSettings* self);
		static void InternalSetEnableHdr(ScriptRenderSettings* self, bool value);
		static bool InternalGetEnableLighting(ScriptRenderSettings* self);
		static void InternalSetEnableLighting(ScriptRenderSettings* self, bool value);
		static bool InternalGetEnableShadows(ScriptRenderSettings* self);
		static void InternalSetEnableShadows(ScriptRenderSettings* self, bool value);
		static bool InternalGetEnableVelocityBuffer(ScriptRenderSettings* self);
		static void InternalSetEnableVelocityBuffer(ScriptRenderSettings* self, bool value);
		static MonoObject* InternalGetShadowSettings(ScriptRenderSettings* self);
		static void InternalSetShadowSettings(ScriptRenderSettings* self, MonoObject* value);
		static bool InternalGetEnableIndirectLighting(ScriptRenderSettings* self);
		static void InternalSetEnableIndirectLighting(ScriptRenderSettings* self, bool value);
		static bool InternalGetOverlayOnly(ScriptRenderSettings* self);
		static void InternalSetOverlayOnly(ScriptRenderSettings* self, bool value);
		static bool InternalGetEnableSkybox(ScriptRenderSettings* self);
		static void InternalSetEnableSkybox(ScriptRenderSettings* self, bool value);
		static float InternalGetCullDistance(ScriptRenderSettings* self);
		static void InternalSetCullDistance(ScriptRenderSettings* self, float value);
	};
}
