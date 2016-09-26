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

	/** Interop class between C++ & CLR for AudioListener. */
	class BS_SCR_BE_EXPORT ScriptAudioListener : public ScriptObject<ScriptAudioListener>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "NativeAudioListener")

	private:
		ScriptAudioListener(MonoObject* instance, const SPtr<AudioListener>& listener);

		SPtr<AudioListener> mListener;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_CreateInstance(MonoObject* instance);
		static void internal_Destroy(ScriptAudioListener* thisPtr);

		static void internal_SetPosition(ScriptAudioListener* thisPtr, Vector3* position);
		static void internal_SetDirection(ScriptAudioListener* thisPtr, Vector3* direction);
		static void internal_SetUp(ScriptAudioListener* thisPtr, Vector3* up);
		static void internal_SetVelocity(ScriptAudioListener* thisPtr, Vector3* velocity);
	};

	/** @} */
}