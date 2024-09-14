//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"

namespace bs { class GUIClickable; }
namespace bs { struct __GUIContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIClickableWrapperBase : public ScriptGUIElementWrapper
	{
	public:
		using ScriptGUIElementWrapper::ScriptGUIElementWrapper;

		virtual void RegisterEvents();
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

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIClickable : public TScriptGUIElementWrapper<GUIClickable, ScriptGUIClickable, ScriptGUIClickableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIClickable")

		ScriptGUIClickable(GUIClickable* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetContent(ScriptGUIClickableWrapperBase* self, __GUIContentInterop* content);
	};
}
