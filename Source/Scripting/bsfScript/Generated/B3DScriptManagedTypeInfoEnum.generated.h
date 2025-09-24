//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoEnum; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoEnum : public TScriptReflectableWrapper<ManagedTypeInfoEnum, ScriptManagedTypeInfoEnum, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoEnum")

		ScriptManagedTypeInfoEnum(const SPtr<ManagedTypeInfoEnum>& nativeObject);
		~ScriptManagedTypeInfoEnum();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static ManagedPrimitiveType InternalGetUnderlyingType(ScriptManagedTypeInfoEnum* self);
		static void InternalSetUnderlyingType(ScriptManagedTypeInfoEnum* self, ManagedPrimitiveType value);
		static MonoString* InternalGetTypeNamespace(ScriptManagedTypeInfoEnum* self);
		static void InternalSetTypeNamespace(ScriptManagedTypeInfoEnum* self, MonoString* value);
		static MonoString* InternalGetTypeName(ScriptManagedTypeInfoEnum* self);
		static void InternalSetTypeName(ScriptManagedTypeInfoEnum* self, MonoString* value);
	};
}
