//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsAudio.h"

namespace BansheeEngine
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/** Interop class between C++ & CLR for AudioDevice. */
	class BS_SCR_BE_EXPORT ScriptAudioDevice : public ScriptObject<ScriptAudioDevice>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "AudioDevice")

		/** Converts a native audio device into its managed counterpart. */
		static MonoObject* create(const AudioDevice& device);

		/** Returns the native audio device. */
		const AudioDevice& getDevice() const { return mDevice; }
	private:
		ScriptAudioDevice(MonoObject* instance, const AudioDevice& device);

		AudioDevice mDevice;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoString* internal_GetName(ScriptAudioDevice* thisPtr);
	};

	/**	Interop class between C++ & CLR for Audio. */
	class BS_SCR_BE_EXPORT ScriptAudio : public ScriptObject<ScriptAudio>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Audio")

	private:
		ScriptAudio(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static void internal_SetVolume(float volume);
		static float internal_GetVolume();
		static void internal_SetActiveDevice(ScriptAudioDevice* device);
		static MonoObject* internal_GetActiveDevice();
		static MonoObject* internal_GetDefaultDevice();
		static MonoArray* internal_GetAllDevices();
	};

	/** @} */
}