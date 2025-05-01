//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace bs { class ManagedTypeInfoPrimitive; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoPrimitive : public TScriptReflectableWrapper<ManagedTypeInfoPrimitive, ScriptManagedTypeInfoPrimitive, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoPrimitive")

		ScriptManagedTypeInfoPrimitive(const SPtr<ManagedTypeInfoPrimitive>& nativeObject);
		~ScriptManagedTypeInfoPrimitive();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static ManagedPrimitiveType InternalGetPrimitiveType(ScriptManagedTypeInfoPrimitive* self);
		static void InternalSetPrimitiveType(ScriptManagedTypeInfoPrimitive* self, ManagedPrimitiveType value);
	};
}
