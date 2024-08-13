//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/GUI/BsScriptGUIElement.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUILabel; }
namespace bs { struct __GUIContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUILabel : public TScriptGUIInteractable<ScriptGUILabel>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUILabel")

		ScriptGUILabel(MonoObject* managedInstance, GUILabel* value);

	private:
		static void InternalSetContent(ScriptGUILabel* self, __GUIContentInterop* content);
		static void InternalCreate(MonoObject* managedInstance, __GUIContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* managedInstance, __GUIContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* managedInstance, MonoArray* options);
	};
}
