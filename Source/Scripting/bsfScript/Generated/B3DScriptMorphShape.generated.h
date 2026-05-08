//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DMorphShapes.h"

namespace b3d { class MorphShape; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphShape : public TScriptReflectableWrapper<MorphShape, ScriptMorphShape>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MorphShape")

		ScriptMorphShape(const TShared<MorphShape>& nativeObject);
		~ScriptMorphShape();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetName(ScriptMorphShape* self);
		static float InternalGetWeight(ScriptMorphShape* self);
	};
}
