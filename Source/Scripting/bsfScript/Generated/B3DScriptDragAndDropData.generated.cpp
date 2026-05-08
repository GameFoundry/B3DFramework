//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptDragAndDropData.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptDragAndDropData::ScriptDragAndDropData(const TShared<DragAndDropData>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptDragAndDropData::~ScriptDragAndDropData()
	{
		UnregisterEvents();
	}

	void ScriptDragAndDropData::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_DragAndDropData", (void*)&ScriptDragAndDropData::InternalDragAndDropData);

	}

	MonoObject* ScriptDragAndDropData::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptDragAndDropData::InternalDragAndDropData(MonoObject* scriptObject)
	{
		TShared<DragAndDropData> nativeObject = B3DMakeShared<DragAndDropData>();
		ScriptObjectWrapper::Create<ScriptDragAndDropData>(nativeObject, scriptObject);
	}

}
