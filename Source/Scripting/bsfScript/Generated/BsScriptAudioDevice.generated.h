//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../Foundation/bsfCore/Audio/BsAudio.h"

namespace bs
{
	struct __AudioDeviceInterop
	{
		MonoString* Name;
	};

	class BS_SCR_BE_EXPORT ScriptAudioDevice : public ScriptObject<ScriptAudioDevice>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "AudioDevice")

		static MonoObject* Box(const __AudioDeviceInterop& value);
		static __AudioDeviceInterop Unbox(MonoObject* value);
		static AudioDevice FromInterop(const __AudioDeviceInterop& value);
		static __AudioDeviceInterop ToInterop(const AudioDevice& value);

	private:
		ScriptAudioDevice(MonoObject* managedInstance);
	};
} // namespace bs
