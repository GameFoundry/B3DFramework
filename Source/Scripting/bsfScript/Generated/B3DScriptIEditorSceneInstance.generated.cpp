//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptIEditorSceneInstance.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
#if B3D_WITH_EDITOR
	ScriptIEditorSceneInstance::ScriptIEditorSceneInstance(const TShared<IEditorSceneInstance>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptIEditorSceneInstance::~ScriptIEditorSceneInstance()
	{
		UnregisterEvents();
	}

	void ScriptIEditorSceneInstance::SetupScriptBindings()
	{

	}

	MonoObject* ScriptIEditorSceneInstance::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
#endif
}
