//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptGUIElementWrapper.h"
#include "BsScriptGUIClickable.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIOptions.h"

namespace bs { class GUIButton; }
namespace bs { struct __GUIContentInterop; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUIButton : public TScriptGUIElementWrapper<GUIButton, ScriptGUIButton, ScriptGUIClickableWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "GUIButton")

		ScriptGUIButton(GUIButton* nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalCreate(MonoObject* scriptObject, __GUIContentInterop* contents, MonoString* styleClass, MonoArray* options);
		static void InternalCreate0(MonoObject* scriptObject, __GUIContentInterop* contents, MonoArray* options);
		static void InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options);
		static void InternalCreate2(MonoObject* scriptObject, MonoArray* options);
	};
}
