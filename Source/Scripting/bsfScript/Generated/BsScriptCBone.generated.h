//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "Wrappers/BsScriptComponent.h"

namespace bs { class CBone; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptBone : public TScriptGameObjectWrapper<CBone, ScriptBone>
	{
	public:
		B3D_SCRIPT_OBJECT_WRAPPER(kEngineAssembly, kEngineNs, "Bone")

		ScriptBone(const GameObjectHandle<CBone>& nativeObject, MonoObject* scriptObject);

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static void InternalSetBoneName(ScriptBone* self, MonoString* name);
		static MonoString* InternalGetBoneName(ScriptBone* self);
	};
}
