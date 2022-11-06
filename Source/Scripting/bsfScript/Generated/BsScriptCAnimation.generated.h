//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "Math/BsAABox.h"
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs { struct __Blend2DInfoInterop; }
namespace bs { class CAnimation; }
namespace bs { struct __Blend1DInfoInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCAnimation : public TScriptComponent<ScriptCAnimation, CAnimation>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Animation")

		ScriptCAnimation(MonoObject* managedInstance, const GameObjectHandle<CAnimation>& value);

	private:
		void ScriptRebuildFloatPropertiesInternal(const ResourceHandle<AnimationClip>& p0);
		void ScriptUpdateFloatPropertiesInternal();
		void ScriptOnEventTriggeredInternal(const ResourceHandle<AnimationClip>& p0, const String& p1);

		typedef void(B3D_THUNKCALL *ScriptRebuildFloatPropertiesInternalThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static ScriptRebuildFloatPropertiesInternalThunkDef ScriptRebuildFloatPropertiesInternalThunk;
		typedef void(B3D_THUNKCALL *ScriptUpdateFloatPropertiesInternalThunkDef) (MonoObject*, MonoException**);
		static ScriptUpdateFloatPropertiesInternalThunkDef ScriptUpdateFloatPropertiesInternalThunk;
		typedef void(B3D_THUNKCALL *ScriptOnEventTriggeredInternalThunkDef) (MonoObject*, MonoObject* p0, MonoString* p1, MonoException**);
		static ScriptOnEventTriggeredInternalThunkDef ScriptOnEventTriggeredInternalThunk;

		static void InternalSetDefaultClip(ScriptCAnimation* thisPtr, MonoObject* clip);
		static MonoObject* InternalGetDefaultClip(ScriptCAnimation* thisPtr);
		static void InternalSetWrapMode(ScriptCAnimation* thisPtr, AnimWrapMode wrapMode);
		static AnimWrapMode InternalGetWrapMode(ScriptCAnimation* thisPtr);
		static void InternalSetSpeed(ScriptCAnimation* thisPtr, float speed);
		static float InternalGetSpeed(ScriptCAnimation* thisPtr);
		static void InternalPlay(ScriptCAnimation* thisPtr, MonoObject* clip);
		static void InternalBlendAdditive(ScriptCAnimation* thisPtr, MonoObject* clip, float weight, float fadeLength, uint32_t layer);
		static void InternalBlend1D(ScriptCAnimation* thisPtr, __Blend1DInfoInterop* info, float t);
		static void InternalBlend2D(ScriptCAnimation* thisPtr, __Blend2DInfoInterop* info, Vector2* t);
		static void InternalCrossFade(ScriptCAnimation* thisPtr, MonoObject* clip, float fadeLength);
		static void InternalSample(ScriptCAnimation* thisPtr, MonoObject* clip, float time);
		static void InternalStop(ScriptCAnimation* thisPtr, uint32_t layer);
		static void InternalStopAll(ScriptCAnimation* thisPtr);
		static bool InternalIsPlaying(ScriptCAnimation* thisPtr);
		static bool InternalGetState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state);
		static void InternalSetState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state);
		static void InternalSetMorphChannelWeight(ScriptCAnimation* thisPtr, MonoString* name, float weight);
		static void InternalSetBounds(ScriptCAnimation* thisPtr, AABox* bounds);
		static void InternalGetBounds(ScriptCAnimation* thisPtr, AABox* __output);
		static void InternalSetUseBounds(ScriptCAnimation* thisPtr, bool enable);
		static bool InternalGetUseBounds(ScriptCAnimation* thisPtr);
		static void InternalSetEnableCull(ScriptCAnimation* thisPtr, bool enable);
		static bool InternalGetEnableCull(ScriptCAnimation* thisPtr);
		static uint32_t InternalGetNumClips(ScriptCAnimation* thisPtr);
		static MonoObject* InternalGetClip(ScriptCAnimation* thisPtr, uint32_t idx);
		static void InternalRefreshClipMappingsInternal(ScriptCAnimation* thisPtr);
		static bool InternalGetGenericCurveValueInternal(ScriptCAnimation* thisPtr, uint32_t curveIdx, float* value);
		static bool InternalTogglePreviewModeInternal(ScriptCAnimation* thisPtr, bool enabled);
	};
}
