//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIVerticalScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIVerticalScrollBar.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIVerticalScrollBar.h"

namespace bs
{
	ScriptGUIVerticalScrollBar::ScriptGUIVerticalScrollBar(MonoObject* managedInstance, GUIVerticalScrollBar* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIVerticalScrollBar::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIVerticalScrollBar::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIVerticalScrollBar::InternalCreate0);

	}

	void ScriptGUIVerticalScrollBar::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIVerticalScrollBar* instance = GUIVerticalScrollBar::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIVerticalScrollBar>())ScriptGUIVerticalScrollBar(managedInstance, instance);
	}

	void ScriptGUIVerticalScrollBar::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIVerticalScrollBar* instance = GUIVerticalScrollBar::Create(vecoptions);
		new (B3DAllocate<ScriptGUIVerticalScrollBar>())ScriptGUIVerticalScrollBar(managedInstance, instance);
	}
}
