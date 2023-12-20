//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUITabButtonContent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContent.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggleGroup.h"
#include "BsScriptGUIToggleGroup.generated.h"

namespace bs
{
	ScriptGUITabButtonContent::ScriptGUITabButtonContent(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUITabButtonContent::InitRuntimeData()
	{ }

	MonoObject*ScriptGUITabButtonContent::Box(const __GUITabButtonContentInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__GUITabButtonContentInterop ScriptGUITabButtonContent::Unbox(MonoObject* value)
	{
		return *(__GUITabButtonContentInterop*)MonoUtil::Unbox(value);
	}

	GUITabButtonContent ScriptGUITabButtonContent::FromInterop(const __GUITabButtonContentInterop& value)
	{
		GUITabButtonContent output;
		output.Index = value.Index;
		GUIContent tmpGeneralContent;
		tmpGeneralContent = ScriptGUIContent::FromInterop(value.GeneralContent);
		output.GeneralContent = tmpGeneralContent;
		SPtr<GUIToggleGroup> tmpToggleGroup;
		ScriptGUIToggleGroup* scriptToggleGroup;
		scriptToggleGroup = ScriptGUIToggleGroup::ToNative(value.ToggleGroup);
		if(scriptToggleGroup != nullptr)
			tmpToggleGroup = scriptToggleGroup->GetInternal();
		output.ToggleGroup = tmpToggleGroup;

		return output;
	}

	__GUITabButtonContentInterop ScriptGUITabButtonContent::ToInterop(const GUITabButtonContent& value)
	{
		__GUITabButtonContentInterop output;
		output.Index = value.Index;
		__GUIContentInterop tmpGeneralContent;
		tmpGeneralContent = ScriptGUIContent::ToInterop(value.GeneralContent);
		output.GeneralContent = tmpGeneralContent;
		MonoObject* tmpToggleGroup;
		tmpToggleGroup = ScriptGUIToggleGroup::Create(value.ToggleGroup);
		output.ToggleGroup = tmpToggleGroup;

		return output;
	}

}
