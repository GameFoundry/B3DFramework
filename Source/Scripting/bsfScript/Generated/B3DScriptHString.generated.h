//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "../../../Foundation/Core/Localization/B3DHString.h"
#include "B3DScriptNonReflectableWrapper.h"

namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptLocString : public TScriptNonReflectableWrapper<HString, ScriptLocString>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "LocString")

		ScriptLocString(const SPtr<HString>& nativeObject);
		~ScriptLocString();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalHString(MonoObject* scriptObject, MonoString* identifier, uint32_t stringTableId);
		static void InternalHString0(MonoObject* scriptObject, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId);
		static void InternalHString1(MonoObject* scriptObject, uint32_t stringTableId);
		static void InternalHString2(MonoObject* scriptObject);
		static MonoString* InternalGetValue(ScriptLocString* self);
		static void InternalSetParameter(ScriptLocString* self, uint32_t index, MonoString* value);
	};
}
