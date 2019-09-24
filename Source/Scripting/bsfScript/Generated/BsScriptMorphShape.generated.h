//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs { class MorphShape; }
namespace bs
{
	class BS_SCR_BE_EXPORT ScriptMorphShape : public TScriptReflectable<ScriptMorphShape, MorphShape>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, ENGINE_NS, "MorphShape")

		ScriptMorphShape(MonoObject* managedInstance, const SPtr<MorphShape>& value);

		static MonoObject* create(const SPtr<MorphShape>& value);

	private:
		static MonoString* Internal_getName(ScriptMorphShape* thisPtr);
		static float Internal_getWeight(ScriptMorphShape* thisPtr);
	};
}
