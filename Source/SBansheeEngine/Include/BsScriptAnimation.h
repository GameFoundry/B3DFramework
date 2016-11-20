//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsAnimation.h"

namespace bs
{
	class ScriptAnimationClip;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Animation. */
	class BS_SCR_BE_EXPORT ScriptAnimation : public ScriptObject <ScriptAnimation>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeAnimation")

		/**	Returns the native wrapped animation. */
		SPtr<Animation> getInternal() const { return mAnimation; }

	private:
		ScriptAnimation(MonoObject* managedInstance);
		~ScriptAnimation();

		/** Callback triggered when animation event triggers. */
		void eventTriggered(const HAnimationClip& clip, const String& name);

		SPtr<Animation> mAnimation;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* instance);
		static void internal_Destroy(ScriptAnimation* thisPtr);

		static void internal_SetWrapMode(ScriptAnimation* thisPtr, AnimWrapMode wrapMode);
		static void internal_SetSpeed(ScriptAnimation* thisPtr, float speed);

		static void internal_Play(ScriptAnimation* thisPtr, ScriptAnimationClip* clip);
		static void internal_BlendAdditive(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, float weight, float fadeLength, UINT32 layer);
		static void internal_Blend1D(ScriptAnimation* thisPtr, MonoObject* info, float t);
		static void internal_Blend2D(ScriptAnimation* thisPtr, MonoObject* info, Vector2* t);
		static void internal_CrossFade(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, float fadeLength);
		static void internal_Sample(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, float time);

		static void internal_Stop(ScriptAnimation* thisPtr, UINT32 layer);
		static void internal_StopAll(ScriptAnimation* thisPtr);
		static bool internal_IsPlaying(ScriptAnimation* thisPtr);

		static bool internal_GetState(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, AnimationClipState* state);
		static void internal_SetState(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, AnimationClipState* state);

		static void internal_SetMorphChannelWeight(ScriptAnimation* thisPtr, UINT32 idx, float weight);

		static UINT32 internal_GetNumClips(ScriptAnimation* thisPtr);
		static MonoObject* internal_GetClip(ScriptAnimation* thisPtr, UINT32 idx);

		static void internal_MapCurveToSceneObject(ScriptAnimation* thisPtr, MonoString* curve, ScriptSceneObject* so);
		static void internal_UnmapSceneObject(ScriptAnimation* thisPtr, ScriptSceneObject* so);

		static void internal_SetBounds(ScriptAnimation* thisPtr, AABox* bounds);
		static void internal_SetCull(ScriptAnimation* thisPtr, bool cull);

		static bool internal_GetGenericCurveValue(ScriptAnimation* thisPtr, UINT32 curveIdx, float* value);

		typedef void(__stdcall *OnEventTriggeredThunkDef) (MonoObject*, MonoObject*, MonoString*, MonoException**);
		static OnEventTriggeredThunkDef sOnEventTriggeredThunk;
	};

	/** Helper class for dealing with BlendSequentialClipInfo structure. */
	class BS_SCR_BE_EXPORT ScriptBlendClipInfo : public ScriptObject<ScriptBlendClipInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "BlendClipInfo")

		/** Converts managed object to its native counterpart. */
		static BlendClipInfo fromManaged(MonoObject* object);

	private:
		ScriptBlendClipInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* clipField;
		static MonoField* positionField;
	};

	/** Helper class for dealing with Blend1DInfo structure. */
	class BS_SCR_BE_EXPORT ScriptBlend1DInfo : public ScriptObject<ScriptBlend1DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Blend1DInfo")

		/** Converts managed object to its native counterpart. */
		static Blend1DInfo fromManaged(MonoObject* object);

	private:
		ScriptBlend1DInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* clipsField;
	};

	/** Helper class for dealing with Blend2DInfo structure. */
	class BS_SCR_BE_EXPORT ScriptBlend2DInfo : public ScriptObject<ScriptBlend2DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Blend2DInfo")

		/** Converts managed object to its native counterpart. */
		static Blend2DInfo fromManaged(MonoObject* object);

	private:
		ScriptBlend2DInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* topLeftClipField;
		static MonoField* topRightClipField;
		static MonoField* botLeftClipField;
		static MonoField* botRightClipField;
	};

	/** @} */
}