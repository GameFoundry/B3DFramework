//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoDictionary; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoDictionary : public TScriptReflectableWrapper<ManagedTypeInfoDictionary, ScriptManagedTypeInfoDictionary, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoDictionary")

		ScriptManagedTypeInfoDictionary(const TShared<ManagedTypeInfoDictionary>& nativeObject);
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
