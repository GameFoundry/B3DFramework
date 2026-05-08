//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIContent.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Localization/B3DHString.h"
#include "B3DScriptHString.generated.h"
#include "../../../Engine/Core/GUI/B3DGUIContent.h"
#include "B3DScriptGUIContentImages.generated.h"

namespace b3d
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
		TShared<HString> tmpText;
		ScriptLocString* scriptObjectWrapperText;
		scriptObjectWrapperText = ScriptLocString::GetScriptObjectWrapper(value.Text);
		if(scriptObjectWrapperText != nullptr)
			tmpText = std::static_pointer_cast<HString>(scriptObjectWrapperText->GetBaseNativeObjectAsShared());
		if(tmpText != nullptr)
		output.Text = *tmpText;
		GUIContentImages tmpImages;
		tmpImages = ScriptGUIContentImages::FromInterop(value.Images);
		output.Images = tmpImages;
		TShared<HString> tmpTooltip;
		ScriptLocString* scriptObjectWrapperTooltip;
		scriptObjectWrapperTooltip = ScriptLocString::GetScriptObjectWrapper(value.Tooltip);
		if(scriptObjectWrapperTooltip != nullptr)
			tmpTooltip = std::static_pointer_cast<HString>(scriptObjectWrapperTooltip->GetBaseNativeObjectAsShared());
		if(tmpTooltip != nullptr)
		output.Tooltip = *tmpTooltip;

		return output;
	}

	__GUIContentInterop ScriptGUIContent::ToInterop(const GUIContent& value)
	{
		__GUIContentInterop output;
		MonoObject* tmpText;
		TShared<HString> tmpTextcopy;
		tmpTextcopy = B3DMakeShared<HString>(value.Text);
		tmpText = ScriptLocString::GetOrCreateScriptObject(tmpTextcopy);
		output.Text = tmpText;
		__GUIContentImagesInterop tmpImages;
		tmpImages = ScriptGUIContentImages::ToInterop(value.Images);
		output.Images = tmpImages;
		MonoObject* tmpTooltip;
		TShared<HString> tmpTooltipcopy;
		tmpTooltipcopy = B3DMakeShared<HString>(value.Tooltip);
		tmpTooltip = ScriptLocString::GetOrCreateScriptObject(tmpTooltipcopy);
		output.Tooltip = tmpTooltip;

		return output;
	}

}
