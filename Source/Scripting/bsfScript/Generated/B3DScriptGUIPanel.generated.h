//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUILayout.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIPanel.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIPanel.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace b3d { class GUIPanel; }
namespace b3d { struct __GUIOptionInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIPanel : public TScriptGUIElementWrapper<GUIPanel, ScriptGUIPanel, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIPanel")

		ScriptGUIPanel(GUIPanel* nativeObject);
		~ScriptGUIPanel();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, GUIPanelContent* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, GUIPanelContent* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
