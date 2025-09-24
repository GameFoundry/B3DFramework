//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptResourceMetaData.generated.h"
#include "../BsManagedResourceMetaData.h"

namespace b3d { class ManagedResourceMetaData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedResourceMetaData : public TScriptReflectableWrapper<ManagedResourceMetaData, ScriptManagedResourceMetaData, ScriptResourceMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedResourceMetaData")

		ScriptManagedResourceMetaData(const SPtr<ManagedResourceMetaData>& nativeObject);
		~ScriptManagedResourceMetaData();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetTypeNamespace(ScriptManagedResourceMetaData* self);
		static void InternalSetTypeNamespace(ScriptManagedResourceMetaData* self, MonoString* value);
		static MonoString* InternalGetTypeName(ScriptManagedResourceMetaData* self);
		static void InternalSetTypeName(ScriptManagedResourceMetaData* self, MonoString* value);
	};
}
