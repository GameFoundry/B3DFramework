//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIProgressBar; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIProgressBar : public TScriptGUIElementWrapper<GUIProgressBar, ScriptGUIProgressBar>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIProgressBar")

		ScriptGUIProgressBar(GUIProgressBar* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetPercent(ScriptGUIProgressBar* self, float percent);
		static float InternalGetPercent(ScriptGUIProgressBar* self);
		static void InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, MonoArray* options);
	};
}
