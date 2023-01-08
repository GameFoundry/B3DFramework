//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIContent.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfCore/Localization/BsHString.h"
#include "BsScriptHString.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIContent.h"
#include "BsScriptGUIContentImages.generated.h"

namespace bs
{
	ScriptGUIContent::ScriptGUIContent(MonoObject* managedInstance)
		:ScriptObject(managedInstance)
	{ }

	void ScriptGUIContent::InitRuntimeData()
	{ }

	MonoObject*ScriptGUIContent::Box(const __GUIContentInterop& value)
	{
		return MonoUtil::Box(metaData.ScriptClass->GetInternalClassInternal(), (void*)&value);
	}

	__GUIContentInterop ScriptGUIContent::Unbox(MonoObject* value)
	{
		return *(__GUIContentInterop*)MonoUtil::Unbox(value);
	}

	GUIContent ScriptGUIContent::FromInterop(const __GUIContentInterop& value)
	{
		GUIContent output;
		SPtr<HString> tmpText;
		ScriptLocString* scriptText;
		scriptText = ScriptLocString::ToNative(value.Text);
		if(scriptText != nullptr)
			tmpText = scriptText->GetInternal();
		if(tmpText != nullptr)
		output.Text = *tmpText;
		GUIContentImages tmpImages;
		tmpImages = ScriptGUIContentImages::FromInterop(value.Images);
		output.Images = tmpImages;
		SPtr<HString> tmpTooltip;
		ScriptLocString* scriptTooltip;
		scriptTooltip = ScriptLocString::ToNative(value.Tooltip);
		if(scriptTooltip != nullptr)
			tmpTooltip = scriptTooltip->GetInternal();
		if(tmpTooltip != nullptr)
		output.Tooltip = *tmpTooltip;

		return output;
	}

	__GUIContentInterop ScriptGUIContent::ToInterop(const GUIContent& value)
	{
		__GUIContentInterop output;
		MonoObject* tmpText;
		SPtr<HString> tmpTextcopy;
		tmpTextcopy = B3DMakeShared<HString>(value.Text);
		tmpText = ScriptLocString::Create(tmpTextcopy);
		output.Text = tmpText;
		__GUIContentImagesInterop tmpImages;
		tmpImages = ScriptGUIContentImages::ToInterop(value.Images);
		output.Images = tmpImages;
		MonoObject* tmpTooltip;
		SPtr<HString> tmpTooltipcopy;
		tmpTooltipcopy = B3DMakeShared<HString>(value.Tooltip);
		tmpTooltip = ScriptLocString::Create(tmpTooltipcopy);
		output.Tooltip = tmpTooltip;

		return output;
	}

}
