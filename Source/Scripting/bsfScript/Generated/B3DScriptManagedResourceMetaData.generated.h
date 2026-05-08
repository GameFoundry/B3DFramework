//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptResourceMetaData.generated.h"
#include "../B3DManagedResourceMetaData.h"

namespace b3d { class ManagedResourceMetaData; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedResourceMetaData : public TScriptReflectableWrapper<ManagedResourceMetaData, ScriptManagedResourceMetaData, ScriptResourceMetaDataWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedResourceMetaData")

		ScriptManagedResourceMetaData(const TShared<ManagedResourceMetaData>& nativeObject);
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
