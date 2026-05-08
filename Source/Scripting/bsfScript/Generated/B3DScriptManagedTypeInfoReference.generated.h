//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoReference; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoReference : public TScriptReflectableWrapper<ManagedTypeInfoReference, ScriptManagedTypeInfoReference, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoReference")

		ScriptManagedTypeInfoReference(const TShared<ManagedTypeInfoReference>& nativeObject);
		~ScriptManagedTypeInfoReference();

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
