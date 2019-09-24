//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs { class MorphChannel; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptMorphChannel : public TScriptReflectable<ScriptMorphChannel, MorphChannel>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MorphChannel")

		ScriptMorphChannel(MonoObject* managedInstance, const SPtr<MorphChannel>& value);

		static MonoObject* create(const SPtr<MorphChannel>& value);

	private:
		static MonoString* Internal_getName(ScriptMorphChannel* thisPtr);
		static MonoArray* Internal_getShapes(ScriptMorphChannel* thisPtr);
	};
}
