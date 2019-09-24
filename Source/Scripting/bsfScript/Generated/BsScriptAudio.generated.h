//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs { class Audio; }
namespace bs { struct __AudioDeviceInterop; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptAudio : public ScriptObject<ScriptAudio>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "Audio")

		ScriptAudio(MonoObject* managedInstance);

	private:
		static void Internal_setVolume(float volume);
		static float Internal_getVolume();
		static void Internal_setPaused(bool paused);
		static bool Internal_isPaused();
		static void Internal_setActiveDevice(__AudioDeviceInterop* device);
		static void Internal_getActiveDevice(__AudioDeviceInterop* __output);
		static void Internal_getDefaultDevice(__AudioDeviceInterop* __output);
		static MonoArray* Internal_getAllDevices();
	};
}
