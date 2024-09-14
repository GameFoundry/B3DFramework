//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Animation/BsMorphShapes.h"

namespace bs { class MorphShape; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphShape : public TScriptReflectableWrapper<MorphShape, ScriptMorphShape>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MorphShape")

		ScriptMorphShape(const SPtr<MorphShape>& nativeObject);

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetName(ScriptMorphShape* self);
		static float InternalGetWeight(ScriptMorphShape* self);
	};
}
