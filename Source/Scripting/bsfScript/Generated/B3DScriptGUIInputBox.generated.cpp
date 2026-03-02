//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIInputBox.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/GUI/B3DGUIInputBox.h"
#include "B3DScriptGUIInputBox.generated.h"
#include "B3DScriptGUIInputBoxContent.generated.h"
#include "B3DScriptGUIOption.generated.h"

namespace b3d
{
	ScriptGUIInputBox::OnValueChangedThunkDefinition ScriptGUIInputBox::OnValueChangedThunk; 
	ScriptGUIInputBox::OnConfirmThunkDefinition ScriptGUIInputBox::OnConfirmThunk; 

	ScriptGUIInputBox::ScriptGUIInputBox(GUIInputBox* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIInputBox::~ScriptGUIInputBox()
	{
		UnregisterEvents();
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
		OnValueChangedConnection = static_cast<GUIInputBox*>(GetNativeObject())->OnValueChanged.Connect([this](const String& p0) { OnValueChanged(p0); });
		OnConfirmConnection = static_cast<GUIInputBox*>(GetNativeObject())->OnConfirm.Connect([this]() { OnConfirm(); });
		ScriptGUIInteractableWrapperBase::RegisterEvents();
	}
	void ScriptGUIInputBox::UnregisterEvents()
	{
		OnValueChangedConnection.Disconnect();
		OnConfirmConnection.Disconnect();
		ScriptGUIInteractableWrapperBase::UnregisterEvents();
	}
	void ScriptGUIInputBox::InternalSetText(ScriptGUIInputBox* self, MonoString* text)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmptext;
		tmptext = MonoUtil::MonoToString(text);
		static_cast<GUIInputBox*>(self->GetNativeObject())->SetText(tmptext);
	}

	MonoString* ScriptGUIInputBox::InternalGetText(ScriptGUIInputBox* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

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
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
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
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
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
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
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
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIInputBox* nativeObject = GUIInputBox::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIInputBox>(nativeObject, scriptObject);
	}
}
