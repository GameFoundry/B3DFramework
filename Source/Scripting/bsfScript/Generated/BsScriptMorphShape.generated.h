//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptReflectable.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs { class MorphShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphShape : public TScriptReflectable<ScriptMorphShape, MorphShape>
	{
	public:
		SCRIPT_OBJ(kEngineAssembly, kEngineNs, "MorphShape")

		ScriptMorphShape(MonoObject* managedInstance, const SPtr<MorphShape>& value);

		static MonoObject* Create(const SPtr<MorphShape>& value);

	private:
		static MonoString* InternalGetName(ScriptMorphShape* thisPtr);
		static float InternalGetWeight(ScriptMorphShape* thisPtr);
	};
}
