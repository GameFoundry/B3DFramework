//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIResizableVerticalScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIVerticalScrollBar.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIVerticalScrollBar.h"

namespace bs
{
	ScriptGUIResizableVerticalScrollBar::ScriptGUIResizableVerticalScrollBar(MonoObject* managedInstance, GUIResizableVerticalScrollBar* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIResizableVerticalScrollBar::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIResizableVerticalScrollBar::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIResizableVerticalScrollBar::InternalCreate0);

	}

	void ScriptGUIResizableVerticalScrollBar::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIResizableVerticalScrollBar* instance = GUIResizableVerticalScrollBar::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIResizableVerticalScrollBar>())ScriptGUIResizableVerticalScrollBar(managedInstance, instance);
	}

	void ScriptGUIResizableVerticalScrollBar::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIResizableVerticalScrollBar* instance = GUIResizableVerticalScrollBar::Create(vecoptions);
		new (B3DAllocate<ScriptGUIResizableVerticalScrollBar>())ScriptGUIResizableVerticalScrollBar(managedInstance, instance);
	}
}
