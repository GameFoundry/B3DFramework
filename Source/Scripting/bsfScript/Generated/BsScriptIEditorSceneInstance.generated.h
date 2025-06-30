//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptReflectableWrapper.h"
#include "../../../Foundation/bsfCore/Scene/BsSceneManager.h"

namespace b3d { class IEditorSceneInstance; }
namespace b3d
{
	class B3D_SCRIPT_INTEROP_EXPORT ScriptIEditorSceneInstanceWrapperBase : public ScriptReflectableWrapper
	{
	public:
		using ScriptReflectableWrapper::ScriptReflectableWrapper;

	};

	class B3D_SCRIPT_INTEROP_EXPORT ScriptIEditorSceneInstance : public TScriptReflectableWrapper<IEditorSceneInstance, ScriptIEditorSceneInstance, ScriptIEditorSceneInstanceWrapperBase>
	{
	public:
		B3D_SCRIPT_TYPE_DEFINITION(kEngineAssembly, kEngineNs, "IEditorSceneInstance")

		ScriptIEditorSceneInstance(const SPtr<IEditorSceneInstance>& nativeObject);
		~ScriptIEditorSceneInstance();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
