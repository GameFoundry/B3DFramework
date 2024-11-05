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
	ScriptGUIContent::ScriptGUIContent()
	{ }

	MonoObject* ScriptGUIContent::Box(const __GUIContentInterop& value)
	{
		return MonoUtil::Box(sInteropMetaData.ScriptClass->GetInternalClass(), (void*)&value);
	}

	__GUIContentInterop ScriptGUIContent::Unbox(MonoObject* value)
	{
		return *(__GUIContentInterop*)MonoUtil::Unbox(value);
	}

	GUIContent ScriptGUIContent::FromInterop(const __GUIContentInterop& value)
	{
		GUIContent output;
		SPtr<HString> tmpText;
		ScriptLocString* scriptWrapperObjectText;
		scriptWrapperObjectText = ScriptLocString::GetScriptObjectWrapper(value.Text);
		if(scriptWrapperObjectText != nullptr)
			tmpText = std::static_pointer_cast<HString>(scriptWrapperObjectText->GetBaseNativeObjectAsShared());
		if(tmpText != nullptr)
		output.Text = *tmpText;
		GUIContentImages tmpImages;
		tmpImages = ScriptGUIContentImages::FromInterop(value.Images);
		output.Images = tmpImages;
		SPtr<HString> tmpTooltip;
		ScriptLocString* scriptWrapperObjectTooltip;
		scriptWrapperObjectTooltip = ScriptLocString::GetScriptObjectWrapper(value.Tooltip);
		if(scriptWrapperObjectTooltip != nullptr)
			tmpTooltip = std::static_pointer_cast<HString>(scriptWrapperObjectTooltip->GetBaseNativeObjectAsShared());
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
		tmpText = ScriptLocString::GetOrCreateScriptObject(tmpTextcopy);
		output.Text = tmpText;
		__GUIContentImagesInterop tmpImages;
		tmpImages = ScriptGUIContentImages::ToInterop(value.Images);
		output.Images = tmpImages;
		MonoObject* tmpTooltip;
		SPtr<HString> tmpTooltipcopy;
		tmpTooltipcopy = B3DMakeShared<HString>(value.Tooltip);
		tmpTooltip = ScriptLocString::GetOrCreateScriptObject(tmpTooltipcopy);
		output.Tooltip = tmpTooltip;

		return output;
	}

}
