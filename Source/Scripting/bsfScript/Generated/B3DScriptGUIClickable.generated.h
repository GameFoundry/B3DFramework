//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptGUIElementWrapper.h"
#include "B3DScriptGUIInteractable.generated.h"
#include "../../../Foundation/Engine/GUI/B3DGUIContent.h"

namespace b3d { struct __GUIContentInterop; }
namespace b3d { class GUIClickable; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIClickableWrapperBase : public ScriptGUIInteractableWrapperBase
	{
	public:
		using ScriptGUIInteractableWrapperBase::ScriptGUIInteractableWrapperBase;

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		void OnClick();
		void OnHover();
		void OnOut();
		void OnDoubleClick();

		typedef void(B3D_THUNKCALL *OnClickThunkDefinition) (MonoObject*, MonoException**);
		static OnClickThunkDefinition OnClickThunk;
		typedef void(B3D_THUNKCALL *OnHoverThunkDefinition) (MonoObject*, MonoException**);
		static OnHoverThunkDefinition OnHoverThunk;
		typedef void(B3D_THUNKCALL *OnOutThunkDefinition) (MonoObject*, MonoException**);
		static OnOutThunkDefinition OnOutThunk;
		typedef void(B3D_THUNKCALL *OnDoubleClickThunkDefinition) (MonoObject*, MonoException**);
		static OnDoubleClickThunkDefinition OnDoubleClickThunk;

		HEvent OnClickConnection;
		HEvent OnHoverConnection;
		HEvent OnOutConnection;
		HEvent OnDoubleClickConnection;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIClickable : public TScriptGUIElementWrapper<GUIClickable, ScriptGUIClickable, ScriptGUIClickableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIClickable")

		ScriptGUIClickable(GUIClickable* nativeObject);
		~ScriptGUIClickable();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetContent(ScriptGUIClickableWrapperBase* self, __GUIContentInterop* content);
	};
}
