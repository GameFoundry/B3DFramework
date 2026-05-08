//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Animation/B3DMorphShapes.h"

namespace b3d { class MorphChannel; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptMorphChannel : public TScriptReflectableWrapper<MorphChannel, ScriptMorphChannel>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "MorphChannel")

		ScriptMorphChannel(const TShared<MorphChannel>& nativeObject);
		~ScriptMorphChannel();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoString* InternalGetName(ScriptMorphChannel* self);
		static MonoArray* InternalGetShapes(ScriptMorphChannel* self);
	};
}
