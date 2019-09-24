//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"

namespace bs { class GUISkin; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptGUISkin : public TScriptResource<ScriptGUISkin, GUISkin>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "GUISkin")

		ScriptGUISkin(MonoObject* managedInstance, const ResourceHandle<GUISkin>& value);

		static MonoObject* createInstance();

	private:
		static MonoObject* Internal_getRef(ScriptGUISkin* thisPtr);

		static bool Internal_hasStyle(ScriptGUISkin* thisPtr, MonoString* name);
		static MonoObject* Internal_getStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType);
		static void Internal_setStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType, MonoObject* style);
		static void Internal_removeStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType);
		static MonoArray* Internal_getStyleNames(ScriptGUISkin* thisPtr);
		static void Internal_create(MonoObject* managedInstance);
	};
}
