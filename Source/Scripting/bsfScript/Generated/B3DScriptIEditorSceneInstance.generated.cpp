//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptIEditorSceneInstance.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace b3d
{
	ScriptIEditorSceneInstance::ScriptIEditorSceneInstance(const SPtr<IEditorSceneInstance>& nativeObject)
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
}
