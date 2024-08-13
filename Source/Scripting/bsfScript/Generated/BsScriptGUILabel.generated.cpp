//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUILabel.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUILabel.h"
#include "BsScriptGUIContent.generated.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUILabel.h"

namespace bs
{
	ScriptGUILabel::ScriptGUILabel(MonoObject* managedInstance, GUILabel* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUILabel::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetContent", (void*)&ScriptGUILabel::InternalSetContent);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUILabel::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUILabel::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUILabel::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUILabel::InternalCreate2);

	}

	void ScriptGUILabel::InternalSetContent(ScriptGUILabel* self, __GUIContentInterop* content)
	{
		GUIContent tmpcontent;
		tmpcontent = ScriptGUIContent::FromInterop(*content);
		static_cast<GUILabel*>(self->GetGuiElement())->SetContent(tmpcontent);
	}

	void ScriptGUILabel::InternalCreate(MonoObject* managedInstance, __GUIContentInterop* contents, MonoString* styleClass, MonoArray* options)
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
		GUILabel* nativeObject = GUILabel::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUILabel>())ScriptGUILabel(managedInstance, nativeObject);
	}

	void ScriptGUILabel::InternalCreate0(MonoObject* managedInstance, __GUIContentInterop* contents, MonoArray* options)
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
		GUILabel* nativeObject = GUILabel::Create(tmpcontents, nativeArrayoptions);
		new (B3DAllocate<ScriptGUILabel>())ScriptGUILabel(managedInstance, nativeObject);
	}

	void ScriptGUILabel::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUILabel* nativeObject = GUILabel::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUILabel>())ScriptGUILabel(managedInstance, nativeObject);
	}

	void ScriptGUILabel::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
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
		GUILabel* nativeObject = GUILabel::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUILabel>())ScriptGUILabel(managedInstance, nativeObject);
	}
}
