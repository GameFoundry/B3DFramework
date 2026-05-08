//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptDragAndDrop.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DDragAndDrop.h"
#include "Reflection/B3DRTTIType.h"
#include "B3DScriptDragAndDropData.generated.h"

namespace b3d
{
	ScriptDragAndDrop::ScriptDragAndDrop()
		:TScriptTypeDefinition()
	{
	}

	void ScriptDragAndDrop::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_StartDrag", (void*)&ScriptDragAndDrop::InternalStartDrag);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsDragInProgress", (void*)&ScriptDragAndDrop::InternalIsDragInProgress);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_IsDropInProgress", (void*)&ScriptDragAndDrop::InternalIsDropInProgress);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDragData", (void*)&ScriptDragAndDrop::InternalGetDragData);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDropData", (void*)&ScriptDragAndDrop::InternalGetDropData);

	}

	void ScriptDragAndDrop::InternalStartDrag(MonoObject* data)
	{
		TShared<DragAndDropData> tmpdata;
		ScriptDragAndDropDataWrapperBase* scriptObjectWrapperdata;
		scriptObjectWrapperdata = (ScriptDragAndDropDataWrapperBase*)ScriptDragAndDropData::GetScriptObjectWrapper(data);
		if(scriptObjectWrapperdata != nullptr)
			tmpdata = std::static_pointer_cast<DragAndDropData>(scriptObjectWrapperdata->GetBaseNativeObjectAsShared());
		DragAndDrop::Instance().StartDrag(tmpdata);
	}

	bool ScriptDragAndDrop::InternalIsDragInProgress()
	{
		bool tmp__output;
		tmp__output = DragAndDrop::Instance().IsDragInProgress();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	bool ScriptDragAndDrop::InternalIsDropInProgress()
	{
		bool tmp__output;
		tmp__output = DragAndDrop::Instance().IsDropInProgress();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	MonoObject* ScriptDragAndDrop::InternalGetDragData()
	{
		TShared<DragAndDropData> tmp__output;
		tmp__output = DragAndDrop::Instance().GetDragData();

		MonoObject* __output;
		__output = ScriptDragAndDropData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}

	MonoObject* ScriptDragAndDrop::InternalGetDropData()
	{
		TShared<DragAndDropData> tmp__output;
		tmp__output = DragAndDrop::Instance().GetDropData();

		MonoObject* __output;
		__output = ScriptDragAndDropData::GetOrCreateScriptObject(tmp__output);

		return __output;
	}
}
