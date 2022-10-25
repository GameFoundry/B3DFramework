//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIToggle.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoMethod.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUIToggle.h"
#include "GUI/BsGUIToggleGroup.h"
#include "GUI/BsGUIOptions.h"
#include "Wrappers/GUI/BsScriptGUIToggleGroup.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

#include "Generated/BsScriptHString.generated.h"
#include "Generated/BsScriptGUIContent.generated.h"
#include "Generated/BsScriptGUIElementStyle.generated.h"

using namespace std::placeholders;

namespace bs
{
	ScriptGUIToggle::OnClickThunkDef ScriptGUIToggle::onClickThunk;
	ScriptGUIToggle::OnHoverThunkDef ScriptGUIToggle::onHoverThunk;
	ScriptGUIToggle::OnOutThunkDef ScriptGUIToggle::onOutThunk;
	ScriptGUIToggle::OnToggledThunkDef ScriptGUIToggle::onToggledThunk;
	ScriptGUIToggle::OnDoubleClickThunkDef ScriptGUIToggle::onDoubleClickThunk;

	ScriptGUIToggle::ScriptGUIToggle(MonoObject* instance, GUIToggle* toggle)
		: TScriptGUIElement(instance, toggle)
	{
	}

	void ScriptGUIToggle::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIToggle::InternalCreateInstance);
		metaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUIToggle::InternalSetContent);
		metaData.ScriptClass->AddInternalCall("Internal_GetValue", (void*)&ScriptGUIToggle::InternalGetValue);
		metaData.ScriptClass->AddInternalCall("Internal_SetValue", (void*)&ScriptGUIToggle::InternalSetValue);
		metaData.ScriptClass->AddInternalCall("Internal_SetTint", (void*)&ScriptGUIToggle::InternalSetTint);

		onClickThunk = (OnClickThunkDef)metaData.ScriptClass->GetMethod("DoOnClick")->GetThunk();
		onHoverThunk = (OnHoverThunkDef)metaData.ScriptClass->GetMethod("DoOnHover")->GetThunk();
		onOutThunk = (OnOutThunkDef)metaData.ScriptClass->GetMethod("DoOnOut")->GetThunk();
		onToggledThunk = (OnToggledThunkDef)metaData.ScriptClass->GetMethod("DoOnToggled", 1)->GetThunk();
		onDoubleClickThunk = (OnDoubleClickThunkDef)metaData.ScriptClass->GetMethod("DoOnDoubleClick")->GetThunk();
	}

	void ScriptGUIToggle::InternalCreateInstance(MonoObject* instance, __GUIContentInterop* content, MonoObject* monoToggleGroup, MonoString* style, MonoArray* guiOptions)
	{
		GUIOptions options;

		ScriptArray scriptArray(guiOptions);
		u32 arrayLen = scriptArray.Size();
		for(u32 i = 0; i < arrayLen; i++)
			options.AddOption(scriptArray.Get<GUIOption>(i));

		ScriptGUIToggleGroup* scriptToggleGroup = nullptr;
		SPtr<GUIToggleGroup> toggleGroup;
		if(monoToggleGroup != nullptr)
		{
			scriptToggleGroup = ScriptGUIToggleGroup::ToNative(monoToggleGroup);
			toggleGroup = scriptToggleGroup->GetInternalValue();
		}

		GUIContent nativeContent = ScriptGUIContent::FromInterop(*content);
		GUIToggle* guiToggle = GUIToggle::Create(nativeContent, toggleGroup, options, MonoUtil::MonoToString(style));

		auto nativeInstance = new(bs_alloc<ScriptGUIToggle>()) ScriptGUIToggle(instance, guiToggle);

		guiToggle->OnClick.Connect(std::bind(&::bs::ScriptGUIToggle::OnClick, nativeInstance));
		guiToggle->OnHover.Connect(std::bind(&::bs::ScriptGUIToggle::OnHover, nativeInstance));
		guiToggle->OnOut.Connect(std::bind(&ScriptGUIToggle::OnOut, nativeInstance));
		guiToggle->OnToggled.Connect(std::bind(&::bs::ScriptGUIToggle::OnToggled, nativeInstance, std::placeholders::_1));
		guiToggle->OnDoubleClick.Connect(std::bind(&::bs::ScriptGUIToggle::OnDoubleClick, nativeInstance));
	}

	void ScriptGUIToggle::InternalSetContent(ScriptGUIToggle* nativeInstance, __GUIContentInterop* content)
	{
		GUIContent nativeContent = ScriptGUIContent::FromInterop(*content);

		GUIToggle* toggle = (GUIToggle*)nativeInstance->GetGuiElement();
		toggle->SetContent(nativeContent);
	}

	bool ScriptGUIToggle::InternalGetValue(ScriptGUIToggle* nativeInstance)
	{
		GUIToggle* toggle = (GUIToggle*)nativeInstance->GetGuiElement();
		return toggle->IsToggled();
	}

	void ScriptGUIToggle::InternalSetValue(ScriptGUIToggle* nativeInstance, bool value)
	{
		GUIToggle* toggle = (GUIToggle*)nativeInstance->GetGuiElement();

		if(value)
			toggle->ToggleOn();
		else
			toggle->ToggleOff();
	}

	void ScriptGUIToggle::InternalSetTint(ScriptGUIToggle* nativeInstance, Color* color)
	{
		GUIToggle* toggle = (GUIToggle*)nativeInstance->GetGuiElement();
		toggle->SetTint(*color);
	}

	void ScriptGUIToggle::OnClick()
	{
		MonoUtil::InvokeThunk(onClickThunk, GetManagedInstance());
	}

	void ScriptGUIToggle::OnHover()
	{
		MonoUtil::InvokeThunk(onHoverThunk, GetManagedInstance());
	}

	void ScriptGUIToggle::OnOut()
	{
		MonoUtil::InvokeThunk(onOutThunk, GetManagedInstance());
	}

	void ScriptGUIToggle::OnToggled(bool toggled)
	{
		MonoUtil::InvokeThunk(onToggledThunk, GetManagedInstance(), toggled);
	}

	void ScriptGUIToggle::OnDoubleClick()
	{
		MonoUtil::InvokeThunk(onDoubleClickThunk, GetManagedInstance());
	}
} // namespace bs
