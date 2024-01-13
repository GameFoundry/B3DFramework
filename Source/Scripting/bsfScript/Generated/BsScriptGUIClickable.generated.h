//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"

namespace bs { class GUIClickable; }
namespace bs { struct __GUIContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIClickableBase : public ScriptGUIInteractableBase
	{
	public:
		ScriptGUIClickableBase(MonoObject* instance);
		virtual ~ScriptGUIClickableBase() {}
		void RegisterEvents(GUIElement* value) override;
		void OnClick();
		void OnHover();
		void OnOut();
		void OnDoubleClick();

		typedef void(B3D_THUNKCALL *OnClickThunkDef) (MonoObject*, MonoException**);
		static OnClickThunkDef OnClickThunk;
		typedef void(B3D_THUNKCALL *OnHoverThunkDef) (MonoObject*, MonoException**);
		static OnHoverThunkDef OnHoverThunk;
		typedef void(B3D_THUNKCALL *OnOutThunkDef) (MonoObject*, MonoException**);
		static OnOutThunkDef OnOutThunk;
		typedef void(B3D_THUNKCALL *OnDoubleClickThunkDef) (MonoObject*, MonoException**);
		static OnDoubleClickThunkDef OnDoubleClickThunk;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIClickable : public TScriptGUIInteractable<ScriptGUIClickable, ScriptGUIClickableBase>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUIClickable")

		ScriptGUIClickable(MonoObject* managedInstance, GUIClickable* value);

	private:
		static void InternalSetContent(ScriptGUIElementBase* thisPtr, __GUIContentInterop* content);
	};
}
