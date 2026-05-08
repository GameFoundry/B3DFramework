//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIToggleContent.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIContent.h"
#include "B3DScriptGUIContent.generated.h"
#include "../../../Engine/Core/GUI/B3DGUIToggleGroup.h"
#include "B3DScriptGUIToggleGroup.generated.h"

namespace b3d
{
	ScriptGUIToggleContent::ScriptGUIToggleContent()
	{ }

	MonoObject* ScriptGUIToggleContent::Box(const __GUIToggleContentInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__GUIToggleContentInterop ScriptGUIToggleContent::Unbox(MonoObject* value)
	{
		return *(__GUIToggleContentInterop*)MonoUtil::Unbox(value);
	}

	GUIToggleContent ScriptGUIToggleContent::FromInterop(const __GUIToggleContentInterop& value)
	{
		GUIToggleContent output;
		GUIContent tmpGeneralContent;
		tmpGeneralContent = ScriptGUIContent::FromInterop(value.GeneralContent);
		output.GeneralContent = tmpGeneralContent;
		TShared<GUIToggleGroup> tmpToggleGroup;
		ScriptGUIToggleGroup* scriptObjectWrapperToggleGroup;
		scriptObjectWrapperToggleGroup = ScriptGUIToggleGroup::GetScriptObjectWrapper(value.ToggleGroup);
		if(scriptObjectWrapperToggleGroup != nullptr)
			tmpToggleGroup = std::static_pointer_cast<GUIToggleGroup>(scriptObjectWrapperToggleGroup->GetBaseNativeObjectAsShared());
		output.ToggleGroup = tmpToggleGroup;

		return output;
	}

	__GUIToggleContentInterop ScriptGUIToggleContent::ToInterop(const GUIToggleContent& value)
	{
		__GUIToggleContentInterop output;
		__GUIContentInterop tmpGeneralContent;
		tmpGeneralContent = ScriptGUIContent::ToInterop(value.GeneralContent);
		output.GeneralContent = tmpGeneralContent;
		MonoObject* tmpToggleGroup;
		tmpToggleGroup = ScriptGUIToggleGroup::GetOrCreateScriptObject(value.ToggleGroup);
		output.ToggleGroup = tmpToggleGroup;

		return output;
	}

}
