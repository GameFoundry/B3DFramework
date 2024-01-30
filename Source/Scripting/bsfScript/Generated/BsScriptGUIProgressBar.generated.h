//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIProgressBar; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIProgressBar : public TScriptGUIInteractable<ScriptGUIProgressBar>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIProgressBar")

		ScriptGUIProgressBar(MonoObject* managedInstance, GUIProgressBar* value);

	private:
		static void InternalSetPercent(ScriptGUIProgressBar* thisPtr, float percent);
		static float InternalGetPercent(ScriptGUIProgressBar* thisPtr);
		static void InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, MonoArray* options);
	};
}
