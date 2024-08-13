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
	ScriptGUIToggle::ScriptGUIToggle(MonoObject* managedInstance, GUIToggle* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIToggle::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIToggle::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIToggle::InternalCreate0);
		metaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIToggle::InternalCreate1);
		metaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIToggle::InternalCreate2);

	}

	void ScriptGUIToggle::InternalCreate(MonoObject* managedInstance, __GUIToggleContentInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUIToggleContent tmpcontents;
		tmpcontents = ScriptGUIToggleContent::FromInterop(*contents);
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
		GUIToggle* nativeObject = GUIToggle::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, nativeObject);
	}

	void ScriptGUIToggle::InternalCreate0(MonoObject* managedInstance, __GUIToggleContentInterop* contents, MonoArray* options)
	{
		GUIToggleContent tmpcontents;
		tmpcontents = ScriptGUIToggleContent::FromInterop(*contents);
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
		GUIToggle* nativeObject = GUIToggle::Create(tmpcontents, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, nativeObject);
	}

	void ScriptGUIToggle::InternalCreate1(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIToggle* nativeObject = GUIToggle::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, nativeObject);
	}

	void ScriptGUIToggle::InternalCreate2(MonoObject* managedInstance, MonoArray* options)
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
		GUIToggle* nativeObject = GUIToggle::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUIToggle>())ScriptGUIToggle(managedInstance, nativeObject);
	}
}
