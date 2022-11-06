//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Audio/BsAudio.h"

namespace bs { class Audio; }
namespace bs { struct __AudioDeviceInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudio : public ScriptObject<ScriptAudio>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "Audio")

		ScriptAudio(MonoObject* managedInstance);

	private:
		static void InternalSetVolume(float volume);
		static float InternalGetVolume();
		static void InternalSetPaused(bool paused);
		static bool InternalIsPaused();
		static void InternalSetActiveDevice(__AudioDeviceInterop* device);
		static void InternalGetActiveDevice(__AudioDeviceInterop* __output);
		static void InternalGetDefaultDevice(__AudioDeviceInterop* __output);
		static MonoArray* InternalGetAllDevices();
	};
}
