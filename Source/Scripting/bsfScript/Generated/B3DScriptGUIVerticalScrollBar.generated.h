//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIScrollBar.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIVerticalScrollBar.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace b3d { struct __GUIOptionInterop; }
namespace b3d { class GUIVerticalScrollBar; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIVerticalScrollBar : public TScriptGUIElementWrapper<GUIVerticalScrollBar, ScriptGUIVerticalScrollBar, ScriptGUIScrollBarWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIVerticalScrollBar")

		ScriptGUIVerticalScrollBar(GUIVerticalScrollBar* nativeObject);
		~ScriptGUIVerticalScrollBar();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, MonoArray* options);
	};
}
