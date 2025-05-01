//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUILayout.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUILayoutY.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUILayoutY; }
namespace bs { struct __GUIOptionInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILayoutY : public TScriptGUIElementWrapper<GUILayoutY, ScriptGUILayoutY, ScriptGUILayoutWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUILayoutY")

		ScriptGUILayoutY(GUILayoutY* nativeObject);
		~ScriptGUILayoutY();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, MonoArray* options);
	};
}
