//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsAnimation.h"

namespace BansheeEngine
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
		static void internal_BlendSequential(ScriptAnimation* thisPtr, MonoObject* info);
		static void internal_Blend1D(ScriptAnimation* thisPtr, MonoObject* info, float t);
		static void internal_Blend2D(ScriptAnimation* thisPtr, MonoObject* info, Vector2* t);
		static void internal_CrossFade(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, float fadeLength);

		static void internal_Stop(ScriptAnimation* thisPtr, UINT32 layer);
		static void internal_StopAll(ScriptAnimation* thisPtr);
		static bool internal_IsPlaying(ScriptAnimation* thisPtr);

		static bool internal_GetState(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, AnimationClipState* state);
		static void internal_SetState(ScriptAnimation* thisPtr, ScriptAnimationClip* clip, AnimationClipState* state);
	};

	/** Helper class for dealing with BlendSequentialInfo structure. */
	class ScriptBlendSequentialInfo : public ScriptObject<ScriptBlendSequentialInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "BlendSequentialInfo")

		/** Converts managed split info to its native counterpart. */
		static BlendSequentialInfo fromManaged(MonoObject* object);

	private:
		ScriptBlendSequentialInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* clipsField;
	};

	/** Helper class for dealing with BlendSequentialClipInfo structure. */
	class ScriptBlendSequentialClipInfo : public ScriptObject<ScriptBlendSequentialClipInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "BlendSequentialClipInfo")

		/** Converts managed split info to its native counterpart. */
		static BlendSequentialClipInfo fromManaged(MonoObject* object);

	private:
		ScriptBlendSequentialClipInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* clipField;
		static MonoField* startTimeField;
		static MonoField* endTimeField;
		static MonoField* fadeTimeField;
	};

	/** Helper class for dealing with Blend1DInfo structure. */
	class ScriptBlend1DInfo : public ScriptObject<ScriptBlend1DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Blend1DInfo")

		/** Converts managed split info to its native counterpart. */
		static Blend1DInfo fromManaged(MonoObject* object);

	private:
		ScriptBlend1DInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* leftClipField;
		static MonoField* rightClipField;
	};

	/** Helper class for dealing with Blend2DInfo structure. */
	class ScriptBlend2DInfo : public ScriptObject<ScriptBlend2DInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Blend2DInfo")

		/** Converts managed split info to its native counterpart. */
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