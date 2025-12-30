//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"

namespace b3d { class AudioListener; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptAudioListener : public TScriptGameObjectWrapper<AudioListener, ScriptAudioListener>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "AudioListener")

		ScriptAudioListener(const TGameObjectHandle<AudioListener>& nativeObject);
		~ScriptAudioListener();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
