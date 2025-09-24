//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIClickable.generated.h"
#include "../../../Foundation/Core/Localization/B3DHString.h"
#include "../../../Foundation/Engine/GUI/B3DGUIOptions.h"
#include "../../../Foundation/Engine/GUI/B3DGUIListBox.h"
#include "../../../Foundation/Engine/GUI/B3DGUIListBox.h"

namespace b3d { struct __GUIOptionInterop; }
namespace b3d { class GUIListBox; }
namespace b3d { struct __GUIListBoxContentInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIListBox : public TScriptGUIElementWrapper<GUIListBox, ScriptGUIListBox, ScriptGUIClickableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIListBox")

		ScriptGUIListBox(GUIListBox* nativeObject);
		~ScriptGUIListBox();

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void OnSelectionToggled(uint32_t p0, bool p1);

		typedef void(B3D_THUNKCALL *OnSelectionToggledThunkDefinition) (MonoObject*, uint32_t p0, bool p1, MonoException**);
		static OnSelectionToggledThunkDefinition OnSelectionToggledThunk;

		HEvent OnSelectionToggledConnection;
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
