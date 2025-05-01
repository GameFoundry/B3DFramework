//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace bs { class ManagedTypeInfoDictionary; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoDictionary : public TScriptReflectableWrapper<ManagedTypeInfoDictionary, ScriptManagedTypeInfoDictionary, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoDictionary")

		ScriptManagedTypeInfoDictionary(const SPtr<ManagedTypeInfoDictionary>& nativeObject);
		~ScriptManagedTypeInfoDictionary();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetKeyType(ScriptManagedTypeInfoDictionary* self);
		static void InternalSetKeyType(ScriptManagedTypeInfoDictionary* self, MonoObject* value);
		static MonoObject* InternalGetValueType(ScriptManagedTypeInfoDictionary* self);
		static void InternalSetValueType(ScriptManagedTypeInfoDictionary* self, MonoObject* value);
	};
}
