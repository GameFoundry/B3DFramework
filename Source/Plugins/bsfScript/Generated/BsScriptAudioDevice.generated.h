//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "../../../../../Foundation/bsfCore/Audio/BsAudio.h"

namespace bs
{
	struct __AudioDeviceInterop
	{
		MonoString* name;
	};

	class BS_SCR_BE_EXPORT ScriptAudioDevice : public ScriptObject<ScriptAudioDevice>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "AudioDevice")

		static MonoObject* box(const __AudioDeviceInterop& value);
		static __AudioDeviceInterop unbox(MonoObject* value);
		static AudioDevice fromInterop(const __AudioDeviceInterop& value);
		static __AudioDeviceInterop toInterop(const AudioDevice& value);

	private:
		ScriptAudioDevice(MonoObject* managedInstance);

	};
}
