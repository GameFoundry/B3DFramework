//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedTypeInfo; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfoWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedTypeInfo : public TScriptReflectableWrapper<ManagedTypeInfo, ScriptManagedTypeInfo, ScriptManagedTypeInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedTypeInfo")

		ScriptManagedTypeInfo(const TShared<ManagedTypeInfo>& nativeObject);
		~ScriptManagedTypeInfo();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static bool InternalMatches(ScriptManagedTypeInfoWrapperBase* self, MonoObject* typeInfo);
		static bool InternalIsTypeLoaded(ScriptManagedTypeInfoWrapperBase* self);
		static MonoReflectionType* InternalGetReflectionType(ScriptManagedTypeInfoWrapperBase* self);
	};
}
