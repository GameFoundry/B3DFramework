//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIInputBox.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIInputBox; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInputBox : public TScriptGUIElementWrapper<GUIInputBox, ScriptGUIInputBox>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIInputBox")

		ScriptGUIInputBox(GUIInputBox* nativeObject);

		static void SetupScriptBindings();

		virtual void RegisterEvents();
		static MonoObject* CreateScriptObject(bool construct);

	private:
		void OnValueChanged(const String& p0);
		void OnConfirm();

		typedef void(B3D_THUNKCALL *OnValueChangedThunkDefinition) (MonoObject*, MonoString* p0, MonoException**);
		static OnValueChangedThunkDefinition OnValueChangedThunk;
		typedef void(B3D_THUNKCALL *OnConfirmThunkDefinition) (MonoObject*, MonoException**);
		static OnConfirmThunkDefinition OnConfirmThunk;

		static void InternalSetText(ScriptGUIInputBox* self, MonoString* text);
		static MonoString* InternalGetText(ScriptGUIInputBox* self);
		static void InternalCreate(MonoObject* scriptObject, GUIInputBoxContent* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, GUIInputBoxContent* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
