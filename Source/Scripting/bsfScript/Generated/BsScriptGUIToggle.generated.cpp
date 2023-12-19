//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggle.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggle.h"
#include "BsScriptGUIOption.generated.h"
#include "BsScriptGUIToggleContent.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggle.h"

namespace bs
{
	ScriptGUIToggle::OnToggledThunkDef ScriptGUIToggle::OnToggledThunk; 

	ScriptGUIToggle::ScriptGUIToggle(MonoObject* managedInstance, GUIToggle* value)
		:TScriptGUIElement(managedInstance, value)
	{
		value->OnToggled.Connect(std::bind(&ScriptGUIToggle::OnToggled, this, std::placeholders::_1));
	}

	void ScriptGUIToggle::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetIsToggled", (void*)&ScriptGUIToggle::InternalSetIsToggled);
		metaData.ScriptClass->AddInternalCall("Internal_IsToggled", (void*)&ScriptGUIToggle::InternalIsToggled);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIToggle::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIToggle::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIToggle::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIToggle::InternalCreate2);

		OnToggledThunk = (OnToggledThunkDef)metaData.ScriptClass->GetMethodExact("Internal_OnToggled", "bool")->GetThunk();
	}

	void ScriptGUIToggle::OnToggled(bool p0)
	{
		MonoUtil::InvokeThunk(OnToggledThunk, GetManagedInstance(), p0);
	}
	void ScriptGUIToggle::InternalSetIsToggled(ScriptGUIToggle* thisPtr, bool isToggled)
	{
		static_cast<GUIToggle*>(thisPtr->GetGuiElement())->SetIsToggled(isToggled);
	}

	bool ScriptGUIToggle::InternalIsToggled(ScriptGUIToggle* thisPtr)
	{
		bool tmp__output;
		tmp__output = static_cast<GUIToggle*>(thisPtr->GetGuiElement())->IsToggled();

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIToggle::InternalCreate(MonoObject* managedInstance, __GUIToggleContentInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUIToggleContent tmpcontents;
		tmpcontents = ScriptGUIToggleContent::FromInterop(*contents);
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
		GUIToggle* instance = GUIToggle::Create(tmpcontents, tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, instance);
	}

	void ScriptGUIToggle::InternalCreate0(MonoObject* managedInstance, __GUIToggleContentInterop* contents, MonoArray* options)
	{
		GUIToggleContent tmpcontents;
		tmpcontents = ScriptGUIToggleContent::FromInterop(*contents);
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
		GUIToggle* instance = GUIToggle::Create(tmpcontents, vecoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, instance);
	}

	void ScriptGUIToggle::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIToggle* instance = GUIToggle::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, instance);
	}

	void ScriptGUIToggle::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
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
		GUIToggle* instance = GUIToggle::Create(vecoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, instance);
	}
}
