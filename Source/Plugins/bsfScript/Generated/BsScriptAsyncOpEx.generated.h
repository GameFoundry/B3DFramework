//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class AsyncOpEx;

	class BS_SCR_BE_EXPORT ScriptAsyncOpEx : public ScriptObject<ScriptAsyncOpEx>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "bs", "AsyncOp")

		ScriptAsyncOpEx(MonoObject* managedInstance, const SPtr<AsyncOpEx>& value);

		SPtr<AsyncOpEx> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<AsyncOpEx>& value);

	private:
		SPtr<AsyncOpEx> mInternal;

		static bool Internal_isComplete(ScriptAsyncOpEx* thisPtr);
		static MonoObject* Internal_getReturnValue(ScriptAsyncOpEx* thisPtr);
		static void Internal_blockUntilComplete(ScriptAsyncOpEx* thisPtr);
	};
}
