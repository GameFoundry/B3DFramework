//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptGUIClickable.generated.h"

namespace bs { class GUIToggleable; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleable : public TScriptGUIElement<ScriptGUIToggleable>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIToggleable")

		ScriptGUIToggleable(MonoObject* managedInstance, GUIToggleable* value);

	private:
		void OnToggled(bool p0);

		typedef void(B3D_THUNKCALL *OnToggledThunkDef) (MonoObject*, bool p0, MonoException**);
		static OnToggledThunkDef OnToggledThunk;

		static void InternalSetIsToggled(ScriptGUIElementBaseTBase* thisPtr, bool isToggled);
		static bool InternalIsToggled(ScriptGUIElementBaseTBase* thisPtr);
	};
}
