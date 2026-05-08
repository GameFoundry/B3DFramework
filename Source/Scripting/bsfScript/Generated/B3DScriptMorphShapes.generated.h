//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DMorphShapes.h"

namespace b3d { class MorphShapes; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphShapes : public TScriptReflectableWrapper<MorphShapes, ScriptMorphShapes>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MorphShapes")

		ScriptMorphShapes(const TShared<MorphShapes>& nativeObject);
		~ScriptMorphShapes();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoArray* InternalGetChannels(ScriptMorphShapes* self);
	};
}
