//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoList; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoList : public TScriptReflectableWrapper<ManagedTypeInfoList, ScriptManagedTypeInfoList, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoList")

		ScriptManagedTypeInfoList(const SPtr<ManagedTypeInfoList>& nativeObject);
		~ScriptManagedTypeInfoList();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetElementType(ScriptManagedTypeInfoList* self);
		static void InternalSetElementType(ScriptManagedTypeInfoList* self, MonoObject* value);
	};
}
