//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptGUISlider.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIVerticalSlider; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIVerticalSlider : public TScriptGUIInteractable<ScriptGUIVerticalSlider>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIVerticalSlider")

		ScriptGUIVerticalSlider(MonoObject* managedInstance, GUIVerticalSlider* value);

	private:
		static void InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, MonoArray* options);
	};
}
