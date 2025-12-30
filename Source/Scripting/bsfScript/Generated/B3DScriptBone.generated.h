//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Wrappers/B3DScriptComponent.h"

namespace b3d { class Bone; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBone : public TScriptGameObjectWrapper<Bone, ScriptBone>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Bone")

		ScriptBone(const TGameObjectHandle<Bone>& nativeObject);
		~ScriptBone();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetBoneName(ScriptBone* self, MonoString* name);
		static MonoString* InternalGetBoneName(ScriptBone* self);
	};
}
