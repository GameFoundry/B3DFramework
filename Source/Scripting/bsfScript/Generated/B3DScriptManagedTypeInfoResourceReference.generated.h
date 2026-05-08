//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "B3DScriptManagedTypeInfo.generated.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfoResourceReference; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoResourceReference : public TScriptReflectableWrapper<ManagedTypeInfoResourceReference, ScriptManagedTypeInfoResourceReference, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfoResourceReference")

		ScriptManagedTypeInfoResourceReference(const TShared<ManagedTypeInfoResourceReference>& nativeObject);
		~ScriptManagedTypeInfoResourceReference();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetResourceType(ScriptManagedTypeInfoResourceReference* self);
		static void InternalSetResourceType(ScriptManagedTypeInfoResourceReference* self, MonoObject* value);
	};
}
