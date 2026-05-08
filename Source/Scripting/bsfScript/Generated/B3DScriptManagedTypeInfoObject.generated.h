//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoObject; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoObject : public TScriptReflectableWrapper<ManagedTypeInfoObject, ScriptManagedTypeInfoObject, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoObject")

		ScriptManagedTypeInfoObject(const TShared<ManagedTypeInfoObject>& nativeObject);
		~ScriptManagedTypeInfoObject();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetTypeNamespace(ScriptManagedTypeInfoObject* self);
		static void InternalSetTypeNamespace(ScriptManagedTypeInfoObject* self, MonoString* value);
		static MonoString* InternalGetTypeName(ScriptManagedTypeInfoObject* self);
		static void InternalSetTypeName(ScriptManagedTypeInfoObject* self, MonoString* value);
		static bool InternalGetIsValueType(ScriptManagedTypeInfoObject* self);
		static void InternalSetIsValueType(ScriptManagedTypeInfoObject* self, bool value);
		static uint32_t InternalGetTypeRTTIId(ScriptManagedTypeInfoObject* self);
		static void InternalSetTypeRTTIId(ScriptManagedTypeInfoObject* self, uint32_t value);
		static ManagedObjectMetaDataFlag InternalGetMetaDataFlags(ScriptManagedTypeInfoObject* self);
		static void InternalSetMetaDataFlags(ScriptManagedTypeInfoObject* self, ManagedObjectMetaDataFlag value);
	};
}
