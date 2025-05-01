//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElement.h"

namespace bs { class GUILayout; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutWrapperBase : public ScriptGUIElementWrapper
	{
	public:
		using ScriptGUIElementWrapper::ScriptGUIElementWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayout : public TScriptGUIElementWrapper<GUILayout, ScriptGUILayout, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUILayout")

		ScriptGUILayout(GUILayout* nativeObject);
		~ScriptGUILayout();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalAddElement(ScriptGUILayoutWrapperBase* self, MonoObject* element);
		static void InternalRemoveElement(ScriptGUILayoutWrapperBase* self, MonoObject* element);
		static void InternalRemoveElementAt(ScriptGUILayoutWrapperBase* self, uint32_t index);
		static void InternalInsertElement(ScriptGUILayoutWrapperBase* self, uint32_t index, MonoObject* element);
		static uint32_t InternalGetChildCount(ScriptGUILayoutWrapperBase* self);
		static MonoObject* InternalGetChild(ScriptGUILayoutWrapperBase* self, uint32_t index);
		static void InternalClear(ScriptGUILayoutWrapperBase* self);
		static void InternalSetEnableCulling(ScriptGUILayoutWrapperBase* self, bool enable);
	};
}
