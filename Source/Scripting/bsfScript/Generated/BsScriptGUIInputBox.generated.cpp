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
	ScriptGUIInputBox::OnValueChangedThunkDefinition ScriptGUIInputBox::OnValueChangedThunk; 
	ScriptGUIInputBox::OnConfirmThunkDefinition ScriptGUIInputBox::OnConfirmThunk; 

	ScriptGUIInputBox::ScriptGUIInputBox(GUIInputBox* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptGUIInputBox::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetText", (void*)&ScriptGUIInputBox::InternalSetText);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetText", (void*)&ScriptGUIInputBox::InternalGetText);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIInputBox::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIInputBox::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIInputBox::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIInputBox::InternalCreate2);

		OnValueChangedThunk = (OnValueChangedThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnValueChanged", "string")->GetThunk();
		OnConfirmThunk = (OnConfirmThunkDefinition)sInteropMetaData.ScriptClass->GetMethodExact("Internal_OnConfirm", "")->GetThunk();
	}

	MonoObject* ScriptGUIInputBox::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIInputBox::OnValueChanged(const String& p0)
	{
		MonoString* tmpp0;
		tmpp0 = MonoUtil::StringToMono(p0);
		MonoUtil::InvokeThunk(OnValueChangedThunk, GetScriptObject(), tmpp0);
	}

	void ScriptGUIInputBox::OnConfirm()
	{
		MonoUtil::InvokeThunk(OnConfirmThunk, GetScriptObject());
	}

	void ScriptGUIInputBox::RegisterEvents()
	{
		static_cast<GUIInputBox*>(GetNativeObject())->OnValueChanged.Connect(std::bind(&ScriptGUIInputBox::OnValueChanged, this, std::placeholders::_1));
		static_cast<GUIInputBox*>(GetNativeObject())->OnConfirm.Connect(std::bind(&ScriptGUIInputBox::OnConfirm, this));
	}
	void ScriptGUIInputBox::InternalSetText(ScriptGUIInputBox* self, MonoString* text)
	{
		String tmptext;
		tmptext = MonoUtil::MonoToString(text);
		static_cast<GUIInputBox*>(self->GetNativeObject())->SetText(tmptext);
	}

	MonoString* ScriptGUIInputBox::InternalGetText(ScriptGUIInputBox* self)
	{
		String tmp__output;
		tmp__output = static_cast<GUIInputBox*>(self->GetNativeObject())->GetText();

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptGUIInputBox::InternalCreate(MonoObject* scriptObject, GUIInputBoxContent* contents, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIInputBox* nativeObject = GUIInputBox::Create(*contents, tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIInputBox>(nativeObject, scriptObject);
	}

	void ScriptGUIInputBox::InternalCreate0(MonoObject* scriptObject, GUIInputBoxContent* contents, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIInputBox* nativeObject = GUIInputBox::Create(*contents, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIInputBox>(nativeObject, scriptObject);
	}

	void ScriptGUIInputBox::InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
	{
		String tmpstyleClass;
		tmpstyleClass = MonoUtil::MonoToString(styleClass);
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIInputBox* nativeObject = GUIInputBox::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIInputBox>(nativeObject, scriptObject);
	}

	void ScriptGUIInputBox::InternalCreate2(MonoObject* scriptObject, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = scriptArrayoptions.Get<GUIOption>(elementIndex);
			}
		}
		GUIInputBox* nativeObject = GUIInputBox::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIInputBox>(nativeObject, scriptObject);
	}
}
