//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIButton.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIButton.h"
#include "BsScriptGUIContent.generated.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIButton.h"

namespace bs
{
	ScriptGUIButton::ScriptGUIButton(MonoObject* managedInstance, GUIButton* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIButton::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIButton::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIButton::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIButton::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIButton::InternalCreate2);

	}

	void ScriptGUIButton::InternalCreate(MonoObject* managedInstance, __GUIContentInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUIContent tmpcontents;
		tmpcontents = ScriptGUIContent::FromInterop(*contents);
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
		GUIButton* instance = GUIButton::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIButton>())ScriptGUIButton(managedInstance, instance);
	}

	void ScriptGUIButton::InternalCreate0(MonoObject* managedInstance, __GUIContentInterop* contents, MonoArray* options)
	{
		GUIContent tmpcontents;
		tmpcontents = ScriptGUIContent::FromInterop(*contents);
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
		GUIButton* instance = GUIButton::Create(tmpcontents, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIButton>())ScriptGUIButton(managedInstance, instance);
	}

	void ScriptGUIButton::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIButton* instance = GUIButton::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIButton>())ScriptGUIButton(managedInstance, instance);
	}

	void ScriptGUIButton::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
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
		GUIButton* instance = GUIButton::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUIButton>())ScriptGUIButton(managedInstance, instance);
	}
}
