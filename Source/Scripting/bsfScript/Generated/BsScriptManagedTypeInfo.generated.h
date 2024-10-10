//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../Serialization/BsManagedTypeInfo.h"

namespace bs { class ManagedTypeInfo; }
namespace bs
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

		ScriptManagedTypeInfo(const SPtr<ManagedTypeInfo>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static bool InternalMatches(ScriptManagedTypeInfoWrapperBase* self, MonoObject* typeInfo);
		static bool InternalIsTypeLoaded(ScriptManagedTypeInfoWrapperBase* self);
	};
}
