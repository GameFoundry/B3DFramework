//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../Serialization/B3DManagedTypeInfo.h"
#include "../Serialization/B3DManagedTypeInfo.h"
#include "../Serialization/B3DManagedTypeInfo.h"

namespace b3d { class ManagedMemberInfo; }
namespace b3d { struct __ManagedMemberStyleInterop; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedMemberInfoWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptManagedMemberInfo : public TScriptReflectableWrapper<ManagedMemberInfo, ScriptManagedMemberInfo, ScriptManagedMemberInfoWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "ManagedMemberInfo")

		ScriptManagedMemberInfo(const TShared<ManagedMemberInfo>& nativeObject);
		~ScriptManagedMemberInfo();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static bool InternalIsSerializable(ScriptManagedMemberInfoWrapperBase* self);
		static void InternalParseStyle(ScriptManagedMemberInfoWrapperBase* self, __ManagedMemberStyleInterop* __output);
		static MonoObject* InternalGetValue(ScriptManagedMemberInfoWrapperBase* self, MonoObject* instance);
		static void InternalSetValue(ScriptManagedMemberInfoWrapperBase* self, MonoObject* instance, MonoObject* value);
		static MonoString* InternalGetName(ScriptManagedMemberInfoWrapperBase* self);
		static void InternalSetName(ScriptManagedMemberInfoWrapperBase* self, MonoString* value);
		static MonoObject* InternalGetTypeInfo(ScriptManagedMemberInfoWrapperBase* self);
		static void InternalSetTypeInfo(ScriptManagedMemberInfoWrapperBase* self, MonoObject* value);
		static ManagedFieldMetaDataFlag InternalGetMetaDataFlags(ScriptManagedMemberInfoWrapperBase* self);
		static void InternalSetMetaDataFlags(ScriptManagedMemberInfoWrapperBase* self, ManagedFieldMetaDataFlag value);
	};
}
