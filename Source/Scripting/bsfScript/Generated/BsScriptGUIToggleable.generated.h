//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptGUIClickable.generated.h"

namespace bs { class GUIToggleable; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleableBase : public ScriptGUIClickableBase
	{
	public:
		ScriptGUIToggleableBase(MonoObject* instance);
		virtual ~ScriptGUIToggleableBase() {}
		void OnToggled(bool p0);

		typedef void(B3D_THUNKCALL *OnToggledThunkDef) (MonoObject*, bool p0, MonoException**);
		static OnToggledThunkDef OnToggledThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggleable : public TScriptGUIInteractable<ScriptGUIToggleable, ScriptGUIToggleableBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIToggleable")

		ScriptGUIToggleable(MonoObject* managedInstance, GUIToggleable* value);

	private:
		static void InternalSetIsToggled(ScriptGUIElementBase* thisPtr, bool isToggled);
		static bool InternalIsToggled(ScriptGUIElementBase* thisPtr);
	};
}
