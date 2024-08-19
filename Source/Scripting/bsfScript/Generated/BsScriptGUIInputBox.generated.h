//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIInputBox.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIInputBox; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInputBox : public TScriptGUIInteractable<ScriptGUIInputBox>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIInputBox")

		ScriptGUIInputBox(MonoObject* managedInstance, GUIInputBox* value);

		void RegisterEvents(GUIElement* value) override;
	private:
		void OnValueChanged(const String& p0);
		void OnConfirm();

		typedef void(B3D_THUNKCALL *OnValueChangedThunkDefinition) (MonoObject*, MonoString* p0, MonoException**);
		static OnValueChangedThunkDefinition OnValueChangedThunk;
		typedef void(B3D_THUNKCALL *OnConfirmThunkDefinition) (MonoObject*, MonoException**);
		static OnConfirmThunkDefinition OnConfirmThunk;

		static void InternalSetText(ScriptGUIInputBox* self, MonoString* text);
		static MonoString* InternalGetText(ScriptGUIInputBox* self);
		static void InternalCreate(MonoObject* managedInstance, GUIInputBoxContent* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, GUIInputBoxContent* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* managedInstance, MonoArray* options);
	};
}
