//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../../../Foundation/bsfCore/Audio/BsAudio.h"
#include "BsScriptTypeDefinition.h"
#include "../../../Foundation/bsfCore/Audio/BsAudio.h"

namespace b3d { struct __AudioDeviceInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudio : public TScriptTypeDefinition<ScriptAudio>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Audio")

		ScriptAudio();

		static void SetupScriptBindings();

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
