//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../BsManagedTypeUtility.h"
#include "BsScriptTypeDefinition.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeUtility : public TScriptTypeDefinition<ScriptManagedTypeUtility>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeUtility")

		ScriptManagedTypeUtility();

		static void SetupScriptBindings();

	private:
		static MonoObject* InternalGetTypeInfo(MonoReflectionType* objectType);
		static MonoObject* InternalGetSerializableObjectInfo(MonoReflectionType* objectType);
		static uint32_t InternalGetRTTITypeId(MonoReflectionType* objectType);
		static MonoObject* InternalCreateSerializableObject(MonoObject* typeInfo);
		static MonoObject* InternalCreateArray(MonoObject* typeInfo, MonoArray* arraySizes);
		static MonoObject* InternalCreateList(MonoObject* typeInfo, uint32_t size);
		static MonoObject* InternalCreateDictionary(MonoObject* typeInfo);
		static MonoObject* InternalCloneObject(MonoObject* original);
		static MonoObject* InternalCreateObjectOfType(MonoReflectionType* type);
	};
}
