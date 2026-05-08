//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "B3DScriptReflectableWrapper.h"
#include "../../../Engine/Core/Scene/B3DSceneInstance.h"

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

		ScriptIEditorSceneInstance(const TShared<IEditorSceneInstance>& nativeObject);
		~ScriptIEditorSceneInstance();

		static void SetupScriptBindings();

		static MonoObject* CreateScriptObject(bool construct);

	private:
	};
}
