//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIVerticalSlider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISlider.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISlider.h"

namespace bs
{
	ScriptGUIVerticalSlider::ScriptGUIVerticalSlider(MonoObject* managedInstance, GUIVerticalSlider* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIVerticalSlider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIVerticalSlider::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIVerticalSlider::InternalCreate0);

	}

	void ScriptGUIVerticalSlider::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIVerticalSlider* instance = GUIVerticalSlider::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIVerticalSlider>())ScriptGUIVerticalSlider(managedInstance, instance);
	}

	void ScriptGUIVerticalSlider::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIVerticalSlider* instance = GUIVerticalSlider::Create(vecoptions);
		new (B3DAllocate<ScriptGUIVerticalSlider>())ScriptGUIVerticalSlider(managedInstance, instance);
	}
}
