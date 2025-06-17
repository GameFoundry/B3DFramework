//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptResourceWrapper.h"

namespace bs { class Prefab; }
namespace bs
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptPrefab : public TScriptResourceWrapper<Prefab, ScriptPrefab>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "Prefab")

		ScriptPrefab(const TResourceHandle<Prefab>& nativeObject);
		~ScriptPrefab();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
		static MonoObject* InternalGetRef(ScriptPrefab* self);

		static MonoObject* InternalInstantiate(ScriptPrefab* self, MonoObject* sceneInstance);
		static void InternalCreate(MonoObject* scriptObject, MonoObject* sceneObject);
	};
}
