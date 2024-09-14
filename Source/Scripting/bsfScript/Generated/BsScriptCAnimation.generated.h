//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "Math/BsAABox.h"

namespace bs { struct __Blend2DInfoInterop; }
namespace bs { class CAnimation; }
namespace bs { struct __Blend1DInfoInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimation : public TScriptGameObjectWrapper<CAnimation, ScriptAnimation>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Animation")

		ScriptAnimation(const GameObjectHandle<CAnimation>& nativeObject);

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void ScriptRebuildFloatPropertiesInternal(const TResourceHandle<AnimationClip>& p0);
		void ScriptUpdateFloatPropertiesInternal();
		void ScriptOnEventTriggeredInternal(const TResourceHandle<AnimationClip>& p0, const String& p1);

		typedef void(B3D_THUNKCALL *ScriptRebuildFloatPropertiesInternalThunkDefinition) (MonoObject*, MonoObject* p0, MonoException**);
		static ScriptRebuildFloatPropertiesInternalThunkDefinition ScriptRebuildFloatPropertiesInternalThunk;
		typedef void(B3D_THUNKCALL *ScriptUpdateFloatPropertiesInternalThunkDefinition) (MonoObject*, MonoException**);
		static ScriptUpdateFloatPropertiesInternalThunkDefinition ScriptUpdateFloatPropertiesInternalThunk;
		typedef void(B3D_THUNKCALL *ScriptOnEventTriggeredInternalThunkDefinition) (MonoObject*, MonoObject* p0, MonoString* p1, MonoException**);
		static ScriptOnEventTriggeredInternalThunkDefinition ScriptOnEventTriggeredInternalThunk;

		static void InternalSetDefaultClip(ScriptAnimation* self, MonoObject* clip);
		static MonoObject* InternalGetDefaultClip(ScriptAnimation* self);
		static void InternalSetWrapMode(ScriptAnimation* self, AnimWrapMode wrapMode);
		static AnimWrapMode InternalGetWrapMode(ScriptAnimation* self);
		static void InternalSetSpeed(ScriptAnimation* self, float speed);
		static float InternalGetSpeed(ScriptAnimation* self);
		static void InternalPlay(ScriptAnimation* self, MonoObject* clip);
		static void InternalBlendAdditive(ScriptAnimation* self, MonoObject* clip, float weight, float fadeLength, uint32_t layer);
		static void InternalBlend1D(ScriptAnimation* self, __Blend1DInfoInterop* info, float t);
		static void InternalBlend2D(ScriptAnimation* self, __Blend2DInfoInterop* info, TVector2<float>* t);
		static void InternalCrossFade(ScriptAnimation* self, MonoObject* clip, float fadeLength);
		static void InternalSample(ScriptAnimation* self, MonoObject* clip, float time);
		static void InternalStop(ScriptAnimation* self, uint32_t layer);
		static void InternalStopAll(ScriptAnimation* self);
		static bool InternalIsPlaying(ScriptAnimation* self);
		static bool InternalGetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* state);
		static void InternalSetState(ScriptAnimation* self, MonoObject* clip, AnimationClipState* state);
		static void InternalSetMorphChannelWeight(ScriptAnimation* self, MonoString* name, float weight);
		static void InternalSetBounds(ScriptAnimation* self, AABox* bounds);
		static void InternalGetBounds(ScriptAnimation* self, AABox* __output);
		static void InternalSetUseBounds(ScriptAnimation* self, bool enable);
		static bool InternalGetUseBounds(ScriptAnimation* self);
		static void InternalSetEnableCull(ScriptAnimation* self, bool enable);
		static bool InternalGetEnableCull(ScriptAnimation* self);
		static uint32_t InternalGetNumClips(ScriptAnimation* self);
		static MonoObject* InternalGetClip(ScriptAnimation* self, uint32_t idx);
		static void InternalRefreshClipMappingsInternal(ScriptAnimation* self);
		static bool InternalGetGenericCurveValueInternal(ScriptAnimation* self, uint32_t curveIdx, float* value);
		static bool InternalTogglePreviewModeInternal(ScriptAnimation* self, bool enabled);
	};
}
