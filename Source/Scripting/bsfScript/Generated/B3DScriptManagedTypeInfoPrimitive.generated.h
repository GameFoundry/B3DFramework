//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoPrimitive; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoPrimitive : public TScriptReflectableWrapper<ManagedTypeInfoPrimitive, ScriptManagedTypeInfoPrimitive, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoPrimitive")

		ScriptManagedTypeInfoPrimitive(const TShared<ManagedTypeInfoPrimitive>& nativeObject);
		~ScriptManagedTypeInfoPrimitive();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static ManagedPrimitiveType InternalGetPrimitiveType(ScriptManagedTypeInfoPrimitive* self);
		static void InternalSetPrimitiveType(ScriptManagedTypeInfoPrimitive* self, ManagedPrimitiveType value);
	};
}
