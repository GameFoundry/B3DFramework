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
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAnimation : public TScriptComponent<ScriptAnimation, CAnimation>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Animation")

		ScriptAnimation(MonoObject* managedInstance, const GameObjectHandle<CAnimation>& value);

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

		static void InternalSetDefaultClip(ScriptAnimation* thisPtr, MonoObject* clip);
		static MonoObject* InternalGetDefaultClip(ScriptAnimation* thisPtr);
		static void InternalSetWrapMode(ScriptAnimation* thisPtr, AnimWrapMode wrapMode);
		static AnimWrapMode InternalGetWrapMode(ScriptAnimation* thisPtr);
		static void InternalSetSpeed(ScriptAnimation* thisPtr, float speed);
		static float InternalGetSpeed(ScriptAnimation* thisPtr);
		static void InternalPlay(ScriptAnimation* thisPtr, MonoObject* clip);
		static void InternalBlendAdditive(ScriptAnimation* thisPtr, MonoObject* clip, float weight, float fadeLength, uint32_t layer);
		static void InternalBlend1D(ScriptAnimation* thisPtr, __Blend1DInfoInterop* info, float t);
		static void InternalBlend2D(ScriptAnimation* thisPtr, __Blend2DInfoInterop* info, Vector2* t);
		static void InternalCrossFade(ScriptAnimation* thisPtr, MonoObject* clip, float fadeLength);
		static void InternalSample(ScriptAnimation* thisPtr, MonoObject* clip, float time);
		static void InternalStop(ScriptAnimation* thisPtr, uint32_t layer);
		static void InternalStopAll(ScriptAnimation* thisPtr);
		static bool InternalIsPlaying(ScriptAnimation* thisPtr);
		static bool InternalGetState(ScriptAnimation* thisPtr, MonoObject* clip, AnimationClipState* state);
		static void InternalSetState(ScriptAnimation* thisPtr, MonoObject* clip, AnimationClipState* state);
		static void InternalSetMorphChannelWeight(ScriptAnimation* thisPtr, MonoString* name, float weight);
		static void InternalSetBounds(ScriptAnimation* thisPtr, AABox* bounds);
		static void InternalGetBounds(ScriptAnimation* thisPtr, AABox* __output);
		static void InternalSetUseBounds(ScriptAnimation* thisPtr, bool enable);
		static bool InternalGetUseBounds(ScriptAnimation* thisPtr);
		static void InternalSetEnableCull(ScriptAnimation* thisPtr, bool enable);
		static bool InternalGetEnableCull(ScriptAnimation* thisPtr);
		static uint32_t InternalGetNumClips(ScriptAnimation* thisPtr);
		static MonoObject* InternalGetClip(ScriptAnimation* thisPtr, uint32_t idx);
		static void InternalRefreshClipMappingsInternal(ScriptAnimation* thisPtr);
		static bool InternalGetGenericCurveValueInternal(ScriptAnimation* thisPtr, uint32_t curveIdx, float* value);
		static bool InternalTogglePreviewModeInternal(ScriptAnimation* thisPtr, bool enabled);
	};
}
