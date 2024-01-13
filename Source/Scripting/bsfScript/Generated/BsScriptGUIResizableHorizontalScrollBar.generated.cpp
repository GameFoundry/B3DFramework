//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIResizableHorizontalScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIHorizontalScrollBar.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIHorizontalScrollBar.h"

namespace bs
{
	ScriptGUIResizableHorizontalScrollBar::ScriptGUIResizableHorizontalScrollBar(MonoObject* managedInstance, GUIResizableHorizontalScrollBar* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIResizableHorizontalScrollBar::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIResizableHorizontalScrollBar::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIResizableHorizontalScrollBar::InternalCreate0);

	}

	void ScriptGUIResizableHorizontalScrollBar::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIResizableHorizontalScrollBar* instance = GUIResizableHorizontalScrollBar::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIResizableHorizontalScrollBar>())ScriptGUIResizableHorizontalScrollBar(managedInstance, instance);
	}

	void ScriptGUIResizableHorizontalScrollBar::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIResizableHorizontalScrollBar* instance = GUIResizableHorizontalScrollBar::Create(vecoptions);
		new (B3DAllocate<ScriptGUIResizableHorizontalScrollBar>())ScriptGUIResizableHorizontalScrollBar(managedInstance, instance);
	}
}
