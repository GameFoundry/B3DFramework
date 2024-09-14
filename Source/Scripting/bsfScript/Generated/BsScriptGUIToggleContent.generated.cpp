//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggleContent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContent.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggleGroup.h"
#include "BsScriptGUIToggleGroup.generated.h"

namespace bs
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
		SPtr<GUIToggleGroup> tmpToggleGroup;
		ScriptGUIToggleGroup* scriptWrapperObjectToggleGroup;
		scriptWrapperObjectToggleGroup = ScriptGUIToggleGroup::GetScriptObjectWrapper(value.ToggleGroup);
		if(scriptWrapperObjectToggleGroup != nullptr)
			tmpToggleGroup = std::static_pointer_cast<GUIToggleGroup>(scriptWrapperObjectToggleGroup->GetBaseNativeObjectAsShared());
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
