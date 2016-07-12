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
	};

	/** @} */
}