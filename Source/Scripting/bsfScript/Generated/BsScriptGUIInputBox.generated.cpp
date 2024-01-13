//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIInputBox.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIInputBox.h"
#include "BsScriptGUIInputBoxContent.generated.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIInputBox.h"

namespace bs
{
	ScriptGUIInputBox::OnValueChangedThunkDef ScriptGUIInputBox::OnValueChangedThunk; 
	ScriptGUIInputBox::OnConfirmThunkDef ScriptGUIInputBox::OnConfirmThunk; 

	ScriptGUIInputBox::ScriptGUIInputBox(MonoObject* managedInstance, GUIInputBox* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIInputBox::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptGUIInputBox::InternalSetText);
		metaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptGUIInputBox::InternalGetText);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIInputBox::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIInputBox::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIInputBox::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIInputBox::InternalCreate2);

		OnValueChangedThunk = (OnValueChangedThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnValueChanged", "string")->GetThunk();
		OnConfirmThunk = (OnConfirmThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnConfirm", "")->GetThunk();
	}

	void ScriptGUIInputBox::OnValueChanged(const String& p0)
	{
		MonoString* tmpp0;
		tmpp0 = MonoUtil::StringToMono(p0);
		MonoUtil::InvokeThunk(OnValueChangedThunk, GetManagedInstance(), tmpp0);
	}

	void ScriptGUIInputBox::OnConfirm()
	{
		MonoUtil::InvokeThunk(OnConfirmThunk, GetManagedInstance());
	}

	void ScriptGUIInputBox::RegisterEvents(GUIElement* value)
	{
		static_cast<GUIInputBox*>(value)->OnValueChanged.Connect(std::bind(&ScriptGUIInputBox::OnValueChanged, this, std::placeholders::_1));
		static_cast<GUIInputBox*>(value)->OnConfirm.Connect(std::bind(&ScriptGUIInputBox::OnConfirm, this));
		ScriptGUIElementBase::RegisterEvents(value);
	}
	void ScriptGUIInputBox::InternalSetText(ScriptGUIInputBox* thisPtr, MonoString* text)
	{
		String tmptext;
		tmptext = MonoUtil::MonoToString(text);
		static_cast<GUIInputBox*>(thisPtr->GetGuiElement())->SetText(tmptext);
	}

	MonoString* ScriptGUIInputBox::InternalGetText(ScriptGUIInputBox* thisPtr)
	{
		String tmp__output;
		tmp__output = static_cast<GUIInputBox*>(thisPtr->GetGuiElement())->GetText();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptGUIInputBox::InternalCreate(MonoObject* managedInstance, GUIInputBoxContent* contents, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIInputBox* instance = GUIInputBox::Create(*contents, tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIInputBox>())ScriptGUIInputBox(managedInstance, instance);
	}

	void ScriptGUIInputBox::InternalCreate0(MonoObject* managedInstance, GUIInputBoxContent* contents, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIInputBox* instance = GUIInputBox::Create(*contents, vecoptions);
		new (B3DAllocate<ScriptGUIInputBox>())ScriptGUIInputBox(managedInstance, instance);
	}

	void ScriptGUIInputBox::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIInputBox* instance = GUIInputBox::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIInputBox>())ScriptGUIInputBox(managedInstance, instance);
	}

	void ScriptGUIInputBox::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> vecoptions;
		if(options != nullptr)
		{
			ScriptArray arrayoptions(options);
			vecoptions.resize(arrayoptions.Size());
			for(int i = 0; i < (int)arrayoptions.Size(); i++)
			{
				vecoptions[i] = arrayoptions.Get<GUIOption>(i);
			}
		}
		GUIInputBox* instance = GUIInputBox::Create(vecoptions);
		new (B3DAllocate<ScriptGUIInputBox>())ScriptGUIInputBox(managedInstance, instance);
	}
}
