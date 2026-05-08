//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedObjectInfo; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedObjectInfo : public TScriptReflectableWrapper<ManagedObjectInfo, ScriptManagedObjectInfo>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedObjectInfo")

		ScriptManagedObjectInfo(const TShared<ManagedObjectInfo>& nativeObject);
		~ScriptManagedObjectInfo();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoReflectionType* InternalGetReflectionType(ScriptManagedObjectInfo* self);
		static MonoObject* InternalGetTypeInfo(ScriptManagedObjectInfo* self);
		static void InternalSetTypeInfo(ScriptManagedObjectInfo* self, MonoObject* value);
		static MonoArray* InternalGetMembers(ScriptManagedObjectInfo* self);
		static void InternalSetMembers(ScriptManagedObjectInfo* self, MonoArray* value);
		static MonoObject* InternalGetBaseClass(ScriptManagedObjectInfo* self);
		static void InternalSetBaseClass(ScriptManagedObjectInfo* self, MonoObject* value);
	};
}
