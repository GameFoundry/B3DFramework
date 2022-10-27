//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs
{
	class MorphShapes;
}

namespace bs
{
	class BS_SCR_BE_EXPORT ScriptMorphShapes : public TScriptReflectable<ScriptMorphShapes, MorphShapes>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MorphShapes")

		ScriptMorphShapes(MonoObject* managedInstance, const SPtr<MorphShapes>& value);

		static MonoObject* Create(const SPtr<MorphShapes>& value);

	private:
		static MonoArray* InternalGetChannels(ScriptMorphShapes* thisPtr);
	};
} // namespace bs
