//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIClickable.generated.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIListBox.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIListBox; }
namespace bs { struct __GUIListBoxContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIListBox : public TScriptGUIElementWrapper<GUIListBox, ScriptGUIListBox, ScriptGUIClickableWrapperBase>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "GUIListBox")

		ScriptGUIListBox(GUIListBox* nativeObject);

		virtual void RegisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void OnSelectionToggled(uint32_t p0, bool p1);

		typedef void(B3D_THUNKCALL *OnSelectionToggledThunkDefinition) (MonoObject*, uint32_t p0, bool p1, MonoException**);
		static OnSelectionToggledThunkDefinition OnSelectionToggledThunk;

		static bool InternalIsMultiselect(ScriptGUIListBox* self);
		static void InternalSetElements(ScriptGUIListBox* self, MonoArray* elements);
		static void InternalSelectElement(ScriptGUIListBox* self, uint32_t index);
		static void InternalDeselectElement(ScriptGUIListBox* self, uint32_t index);
		static uint32_t InternalGetSelectedElementIndex(ScriptGUIListBox* self);
		static MonoArray* InternalGetElementStates(ScriptGUIListBox* self);
		static void InternalSetElementStates(ScriptGUIListBox* self, MonoArray* states);
		static void InternalCreate(MonoObject* scriptObject, __GUIListBoxContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, __GUIListBoxContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
