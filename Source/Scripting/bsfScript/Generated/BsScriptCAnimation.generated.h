//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"
#include "Math/BsAABox.h"
#include "Math/BsVector2.h"
#include "../../../Foundation/bsfCore/Animation/BsAnimation.h"

namespace bs { struct __Blend2DInfoInterop; }
namespace bs { class CAnimation; }
namespace bs { struct __Blend1DInfoInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptCAnimation : public TScriptComponent<ScriptCAnimation, CAnimation>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Animation")

		ScriptCAnimation(MonoObject* managedInstance, const GameObjectHandle<CAnimation>& value);

	private:
		void _scriptRebuildFloatProperties(const ResourceHandle<AnimationClip>& p0);
		void _scriptUpdateFloatProperties();
		void _scriptOnEventTriggered(const ResourceHandle<AnimationClip>& p0, const String& p1);

		typedef void(BS_THUNKCALL *_scriptRebuildFloatPropertiesThunkDef) (MonoObject*, MonoObject* p0, MonoException**);
		static _scriptRebuildFloatPropertiesThunkDef _scriptRebuildFloatPropertiesThunk;
		typedef void(BS_THUNKCALL *_scriptUpdateFloatPropertiesThunkDef) (MonoObject*, MonoException**);
		static _scriptUpdateFloatPropertiesThunkDef _scriptUpdateFloatPropertiesThunk;
		typedef void(BS_THUNKCALL *_scriptOnEventTriggeredThunkDef) (MonoObject*, MonoObject* p0, MonoString* p1, MonoException**);
		static _scriptOnEventTriggeredThunkDef _scriptOnEventTriggeredThunk;

		static void Internal_setDefaultClip(ScriptCAnimation* thisPtr, MonoObject* clip);
		static MonoObject* Internal_getDefaultClip(ScriptCAnimation* thisPtr);
		static void Internal_setWrapMode(ScriptCAnimation* thisPtr, AnimWrapMode wrapMode);
		static AnimWrapMode Internal_getWrapMode(ScriptCAnimation* thisPtr);
		static void Internal_setSpeed(ScriptCAnimation* thisPtr, float speed);
		static float Internal_getSpeed(ScriptCAnimation* thisPtr);
		static void Internal_play(ScriptCAnimation* thisPtr, MonoObject* clip);
		static void Internal_blendAdditive(ScriptCAnimation* thisPtr, MonoObject* clip, float weight, float fadeLength, uint32_t layer);
		static void Internal_blend1D(ScriptCAnimation* thisPtr, __Blend1DInfoInterop* info, float t);
		static void Internal_blend2D(ScriptCAnimation* thisPtr, __Blend2DInfoInterop* info, Vector2* t);
		static void Internal_crossFade(ScriptCAnimation* thisPtr, MonoObject* clip, float fadeLength);
		static void Internal_sample(ScriptCAnimation* thisPtr, MonoObject* clip, float time);
		static void Internal_stop(ScriptCAnimation* thisPtr, uint32_t layer);
		static void Internal_stopAll(ScriptCAnimation* thisPtr);
		static bool Internal_isPlaying(ScriptCAnimation* thisPtr);
		static bool Internal_getState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state);
		static void Internal_setState(ScriptCAnimation* thisPtr, MonoObject* clip, AnimationClipState* state);
		static void Internal_setMorphChannelWeight(ScriptCAnimation* thisPtr, MonoString* name, float weight);
		static void Internal_setBounds(ScriptCAnimation* thisPtr, AABox* bounds);
		static void Internal_getBounds(ScriptCAnimation* thisPtr, AABox* __output);
		static void Internal_setUseBounds(ScriptCAnimation* thisPtr, bool enable);
		static bool Internal_getUseBounds(ScriptCAnimation* thisPtr);
		static void Internal_setEnableCull(ScriptCAnimation* thisPtr, bool enable);
		static bool Internal_getEnableCull(ScriptCAnimation* thisPtr);
		static uint32_t Internal_getNumClips(ScriptCAnimation* thisPtr);
		static MonoObject* Internal_getClip(ScriptCAnimation* thisPtr, uint32_t idx);
		static void Internal__refreshClipMappings(ScriptCAnimation* thisPtr);
		static bool Internal__getGenericCurveValue(ScriptCAnimation* thisPtr, uint32_t curveIdx, float* value);
		static bool Internal__togglePreviewMode(ScriptCAnimation* thisPtr, bool enabled);
	};
}
