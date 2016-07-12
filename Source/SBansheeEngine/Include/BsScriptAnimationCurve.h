//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	
	/** Interop class between C++ & CLR for KeyFrame. */
	class BS_SCR_BE_EXPORT ScriptKeyFrame : public ScriptObject<ScriptKeyFrame>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "KeyFrame")

	private:
		ScriptKeyFrame(MonoObject* instance);
	};

	/** Interop class between C++ & CLR for AnimationCurve. */
	class BS_SCR_BE_EXPORT ScriptAnimationCurve : public ScriptObject<ScriptAnimationCurve>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AnimationCurve")

	private:
		ScriptAnimationCurve(MonoObject* instance, const SPtr<TAnimationCurve<float>>& curve);

		SPtr<TAnimationCurve<float>> mCurve;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_Create(MonoObject* instance, MonoArray* keyframes);
		static MonoArray* internal_GetKeyFrames(ScriptAnimationCurve* thisPtr);
		static void internal_SetKeyFrames(ScriptAnimationCurve* thisPtr, MonoArray* keyframes);
		static float internal_Evaluate(ScriptAnimationCurve* thisPtr, float time, bool loop);
	};

	/** @} */
}