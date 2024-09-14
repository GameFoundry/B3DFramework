//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIScrollBar.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIHorizontalScrollBar; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIHorizontalScrollBar : public TScriptGUIElementWrapper<GUIHorizontalScrollBar, ScriptGUIHorizontalScrollBar, ScriptGUIScrollBarWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIHorizontalScrollBar")

		ScriptGUIHorizontalScrollBar(GUIHorizontalScrollBar* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, MonoArray* options);
	};
}
