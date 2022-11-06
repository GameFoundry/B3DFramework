//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs { class CAudioListener; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptCAudioListener : public TScriptComponent<ScriptCAudioListener, CAudioListener>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "AudioListener")

		ScriptCAudioListener(MonoObject* managedInstance, const GameObjectHandle<CAudioListener>& value);

	private:
	};
}
