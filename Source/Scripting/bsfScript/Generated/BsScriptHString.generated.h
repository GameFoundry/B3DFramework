//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs { class HString; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptHString : public ScriptObject<ScriptHString>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "LocString")

		ScriptHString(MonoObject* managedInstance, const SPtr<HString>& value);

		SPtr<HString> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<HString>& value);

	private:
		SPtr<HString> mInternal;

		static void Internal_HString(MonoObject* managedInstance, MonoString* identifier, uint32_t stringTableId);
		static void Internal_HString0(MonoObject* managedInstance, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId);
		static void Internal_HString1(MonoObject* managedInstance, uint32_t stringTableId);
		static void Internal_HString2(MonoObject* managedInstance);
		static MonoString* Internal_getValue(ScriptHString* thisPtr);
		static void Internal_setParameter(ScriptHString* thisPtr, uint32_t idx, MonoString* value);
	};
}
