//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"

namespace bs
{
	class MorphChannel;

	class BS_SCR_BE_EXPORT ScriptMorphChannel : public ScriptObject<ScriptMorphChannel>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MorphChannel")

		ScriptMorphChannel(MonoObject* managedInstance, const SPtr<MorphChannel>& value);

		SPtr<MorphChannel> getInternal() const { return mInternal; }
		static MonoObject* create(const SPtr<MorphChannel>& value);

	private:
		SPtr<MorphChannel> mInternal;

		static MonoString* Internal_getName(ScriptMorphChannel* thisPtr);
		static MonoArray* Internal_getShapes(ScriptMorphChannel* thisPtr);
	};
}
