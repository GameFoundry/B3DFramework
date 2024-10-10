//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "BsScriptManagedTypeInfo.generated.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace bs { class ManagedTypeInfoResourceReference; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoResourceReference : public TScriptReflectableWrapper<ManagedTypeInfoResourceReference, ScriptManagedTypeInfoResourceReference, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoResourceReference")

		ScriptManagedTypeInfoResourceReference(const SPtr<ManagedTypeInfoResourceReference>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetResourceType(ScriptManagedTypeInfoResourceReference* self);
		static void InternalSetResourceType(ScriptManagedTypeInfoResourceReference* self, MonoObject* value);
	};
}
