//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "../BsManagedTypeUtility.h"
#include "BsScriptTypeDefinition.h"

namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeUtility : public TScriptTypeDefinition<ScriptManagedTypeUtility>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeUtility")

		ScriptManagedTypeUtility();

		static void SetupScriptBindings();

	private:
		static MonoObject* InternalGetTypeInfo(MonoObject* scriptObject);
		static MonoObject* InternalGetSerializableObjectInfo(MonoObject* scriptObject);
		static MonoObject* InternalCloneObject(MonoObject* original);
		static MonoObject* InternalCreateObjectOfType(MonoReflectionType* type);
	};
}
