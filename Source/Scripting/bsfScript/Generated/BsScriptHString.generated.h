//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class HString;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptHString : public ScriptObject<ScriptHString>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "LocString")

		ScriptHString(MonoObject* managedInstance, const SPtr<HString>& value);

		SPtr<HString> GetInternal() const { return mInternal; }

		static MonoObject* Create(const SPtr<HString>& value);

	private:
		SPtr<HString> mInternal;

		static void InternalHString(MonoObject* managedInstance, MonoString* identifier, uint32_t stringTableId);
		static void InternalHString0(MonoObject* managedInstance, MonoString* identifier, MonoString* defaultString, uint32_t stringTableId);
		static void InternalHString1(MonoObject* managedInstance, uint32_t stringTableId);
		static void InternalHString2(MonoObject* managedInstance);
		static MonoString* InternalGetValue(ScriptHString* thisPtr);
		static void InternalSetParameter(ScriptHString* thisPtr, uint32_t idx, MonoString* value);
	};
} // namespace bs
