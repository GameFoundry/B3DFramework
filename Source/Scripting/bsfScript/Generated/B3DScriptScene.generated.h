//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"

namespace b3d { class Scene; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptScene : public TScriptResourceWrapper<Scene, ScriptScene>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Scene")

		ScriptScene(const TResourceHandle<Scene>& nativeObject);
		~ScriptScene();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptScene* self);

		static MonoObject* InternalInstantiate(ScriptScene* self);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* sceneObject);
	};
}
