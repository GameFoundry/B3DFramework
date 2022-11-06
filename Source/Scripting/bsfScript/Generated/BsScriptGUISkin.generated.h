//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptResource.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIElementStyle.h"

namespace bs { class GUISkin; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptGUISkin : public TScriptResource<ScriptGUISkin, GUISkin>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "GUISkin")

		ScriptGUISkin(MonoObject* managedInstance, const ResourceHandle<GUISkin>& value);

		static MonoObject* CreateInstance();

	private:
		static MonoObject* InternalGetRef(ScriptGUISkin* thisPtr);

		static bool InternalHasStyle(ScriptGUISkin* thisPtr, MonoString* name);
		static MonoObject* InternalGetStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType);
		static void InternalSetStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType, MonoObject* style);
		static void InternalRemoveStyle(ScriptGUISkin* thisPtr, MonoString* guiElemType);
		static MonoArray* InternalGetStyleNames(ScriptGUISkin* thisPtr);
		static void InternalCreate(MonoObject* managedInstance);
	};
}
