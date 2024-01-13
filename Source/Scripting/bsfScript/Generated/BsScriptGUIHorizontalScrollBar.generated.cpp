//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIHorizontalScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIHorizontalScrollBar.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIHorizontalScrollBar.h"

namespace bs
{
	ScriptGUIHorizontalScrollBar::ScriptGUIHorizontalScrollBar(MonoObject* managedInstance, GUIHorizontalScrollBar* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIHorizontalScrollBar::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIHorizontalScrollBar::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIHorizontalScrollBar::InternalCreate0);

	}

	void ScriptGUIHorizontalScrollBar::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIHorizontalScrollBar* instance = GUIHorizontalScrollBar::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIHorizontalScrollBar>())ScriptGUIHorizontalScrollBar(managedInstance, instance);
	}

	void ScriptGUIHorizontalScrollBar::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIHorizontalScrollBar* instance = GUIHorizontalScrollBar::Create(vecoptions);
		new (B3DAllocate<ScriptGUIHorizontalScrollBar>())ScriptGUIHorizontalScrollBar(managedInstance, instance);
	}
}
