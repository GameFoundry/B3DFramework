//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoArray; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoArray : public TScriptReflectableWrapper<ManagedTypeInfoArray, ScriptManagedTypeInfoArray, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoArray")

		ScriptManagedTypeInfoArray(const TShared<ManagedTypeInfoArray>& nativeObject);
		~ScriptManagedTypeInfoArray();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetElementType(ScriptManagedTypeInfoArray* self);
		static void InternalSetElementType(ScriptManagedTypeInfoArray* self, MonoObject* value);
		static uint32_t InternalGetRank(ScriptManagedTypeInfoArray* self);
		static void InternalSetRank(ScriptManagedTypeInfoArray* self, uint32_t value);
	};
}
