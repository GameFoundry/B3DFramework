//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptDragAndDropData.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
	ScriptDragAndDropData::ScriptDragAndDropData(const SPtr<DragAndDropData>& nativeObject)
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
		SPtr<DragAndDropData> nativeObject = B3DMakeShared<DragAndDropData>();
		ScriptObjectWrapper::Create<ScriptDragAndDropData>(nativeObject, scriptObject);
	}

}
