//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "BsScriptGUIClickable.generated.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIListBox.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIListBox; }
namespace bs { struct __GUIListBoxContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIListBox : public TScriptGUIInteractable<ScriptGUIListBox>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIListBox")

		ScriptGUIListBox(MonoObject* managedInstance, GUIListBox* value);

	private:
		void OnSelectionToggled(uint32_t p0, bool p1);

		typedef void(B3D_THUNKCALL *OnSelectionToggledThunkDef) (MonoObject*, uint32_t p0, bool p1, MonoException**);
		static OnSelectionToggledThunkDef OnSelectionToggledThunk;

		static bool InternalIsMultiselect(ScriptGUIListBox* thisPtr);
		static void InternalSetElements(ScriptGUIListBox* thisPtr, MonoArray* elements);
		static void InternalSelectElement(ScriptGUIListBox* thisPtr, uint32_t index);
		static void InternalDeselectElement(ScriptGUIListBox* thisPtr, uint32_t index);
		static MonoArray* InternalGetElementStates(ScriptGUIListBox* thisPtr);
		static void InternalSetElementStates(ScriptGUIListBox* thisPtr, MonoArray* states);
		static void InternalCreate(MonoObject* managedInstance, __GUIListBoxContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, __GUIListBoxContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* managedInstance, MonoArray* options);
	};
}
