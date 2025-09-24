//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIRenderable.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIInteractable.h"

namespace b3d { class GUIInteractable; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInteractableWrapperBase : public ScriptGUIRenderableWrapperBase
	{
	public:
		using ScriptGUIRenderableWrapperBase::ScriptGUIRenderableWrapperBase;

		virtual void RegisterEvents();
		virtual void UnregisterEvents();
		void OnFocusGained();
		void OnFocusLost();

		typedef void(B3D_THUNKCALL *OnFocusGainedThunkDefinition) (MonoObject*, MonoException**);
		static OnFocusGainedThunkDefinition OnFocusGainedThunk;
		typedef void(B3D_THUNKCALL *OnFocusLostThunkDefinition) (MonoObject*, MonoException**);
		static OnFocusLostThunkDefinition OnFocusLostThunk;

		HEvent OnFocusGainedConnection;
		HEvent OnFocusLostConnection;
	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIInteractable : public TScriptGUIElementWrapper<GUIInteractable, ScriptGUIInteractable, ScriptGUIInteractableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIInteractable")

		ScriptGUIInteractable(GUIInteractable* nativeObject);
		~ScriptGUIInteractable();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetFocus(ScriptGUIInteractableWrapperBase* self, bool enabled, bool clear);
		static void InternalSetOptionFlags(ScriptGUIInteractableWrapperBase* self, GUIElementOption options);
		static GUIElementOption InternalGetOptionFlags(ScriptGUIInteractableWrapperBase* self);
		static void InternalSetContextMenu(ScriptGUIInteractableWrapperBase* self, MonoObject* menu);
	};
}
