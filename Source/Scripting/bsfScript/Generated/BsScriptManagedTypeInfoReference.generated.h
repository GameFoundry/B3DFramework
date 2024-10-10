//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace bs { class ManagedTypeInfoReference; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoReference : public TScriptReflectableWrapper<ManagedTypeInfoReference, ScriptManagedTypeInfoReference, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoReference")

		ScriptManagedTypeInfoReference(const SPtr<ManagedTypeInfoReference>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static ManagedReferenceType InternalGetReferenceType(ScriptManagedTypeInfoReference* self);
		static void InternalSetReferenceType(ScriptManagedTypeInfoReference* self, ManagedReferenceType value);
		static uint32_t InternalGetTypeRTTIId(ScriptManagedTypeInfoReference* self);
		static void InternalSetTypeRTTIId(ScriptManagedTypeInfoReference* self, uint32_t value);
		static MonoString* InternalGetTypeNamespace(ScriptManagedTypeInfoReference* self);
		static void InternalSetTypeNamespace(ScriptManagedTypeInfoReference* self, MonoString* value);
		static MonoString* InternalGetTypeName(ScriptManagedTypeInfoReference* self);
		static void InternalSetTypeName(ScriptManagedTypeInfoReference* self, MonoString* value);
	};
}
