//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace b3d { class AudioListener; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioListener : public TScriptGameObjectWrapper<AudioListener, ScriptAudioListener>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AudioListener")

		ScriptAudioListener(const GameObjectHandle<AudioListener>& nativeObject);
		~ScriptAudioListener();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
