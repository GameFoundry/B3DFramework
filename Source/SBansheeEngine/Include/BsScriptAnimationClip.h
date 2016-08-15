//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResource.h"
#include "BsAnimationClip.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for AnimationClip. */
	class BS_SCR_BE_EXPORT ScriptAnimationClip : public TScriptResource<ScriptAnimationClip, AnimationClip>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AnimationClip")

		/**	Creates an empty, uninitialized managed instance of the resource interop object. */
		static MonoObject* createInstance();

	private:
		friend class ScriptResourceManager;

		ScriptAnimationClip(MonoObject* instance, const HAnimationClip& animationClip);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static MonoObject* internal_GetAnimationCurves(ScriptAnimationClip* thisPtr);
		static void internal_SetAnimationCurves(ScriptAnimationClip* thisPtr, MonoObject* curves);
		static MonoArray* internal_GetAnimationEvents(ScriptAnimationClip* thisPtr);
		static void internal_SetAnimationEvents(ScriptAnimationClip* thisPtr, MonoArray* events);
		static float internal_GetLength(ScriptAnimationClip* thisPtr);
	};

	/**	Interop class between C++ & CLR for AnimationEvent. */
	class BS_SCR_BE_EXPORT ScriptAnimationEvent : public ScriptObject<ScriptAnimationEvent>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AnimationEvent")

		/** Converts managed animation event its native counterpart. */
		static AnimationEvent toNative(MonoObject* object);

		/** Converts native animation event to its managed counterpart. */
		static MonoObject* toManaged(const AnimationEvent& event);
	private:
		ScriptAnimationEvent(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sNameField;
		static MonoField* sTimeField;
	};

	/** @} */
}