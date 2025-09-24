//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptObjectWrapper.h"
#include "../../../Foundation/Core/Audio/B3DAudio.h"

namespace b3d
{
	struct __AudioDeviceInterop
	{
		MonoString* Name;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioDevice : public TScriptTypeDefinition<ScriptAudioDevice>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AudioDevice")

		static MonoObject* Box(const __AudioDeviceInterop& value);
		static __AudioDeviceInterop Unbox(MonoObject* value);
		static AudioDevice FromInterop(const __AudioDeviceInterop& value);
		static __AudioDeviceInterop ToInterop(const AudioDevice& value);

	private:
		ScriptAudioDevice();

	};
}
