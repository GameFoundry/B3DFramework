//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptGUIClickable.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggle.h"

namespace bs { class GUIToggle; }
namespace bs { struct __GUIToggleContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIToggle : public TScriptGUIElement<ScriptGUIToggle>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIToggle")

		ScriptGUIToggle(MonoObject* managedInstance, GUIToggle* value);

	private:
		void OnToggled(bool p0);

		typedef void(B3D_THUNKCALL *OnToggledThunkDef) (MonoObject*, bool p0, MonoException**);
		static OnToggledThunkDef OnToggledThunk;

		static void InternalSetIsToggled(ScriptGUIToggle* thisPtr, bool isToggled);
		static bool InternalIsToggled(ScriptGUIToggle* thisPtr);
		static void InternalCreate(MonoObject* managedInstance, __GUIToggleContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, __GUIToggleContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* managedInstance, MonoArray* options);
	};
}
