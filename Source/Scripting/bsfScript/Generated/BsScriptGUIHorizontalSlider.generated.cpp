//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIHorizontalSlider.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISlider.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISlider.h"

namespace bs
{
	ScriptGUIHorizontalSlider::ScriptGUIHorizontalSlider(MonoObject* managedInstance, GUIHorizontalSlider* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIHorizontalSlider::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIHorizontalSlider::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIHorizontalSlider::InternalCreate0);

	}

	void ScriptGUIHorizontalSlider::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIHorizontalSlider* instance = GUIHorizontalSlider::Create(tmpstyleClass, vecoptions);
		new (B3DAllocate<ScriptGUIHorizontalSlider>())ScriptGUIHorizontalSlider(managedInstance, instance);
	}

	void ScriptGUIHorizontalSlider::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIHorizontalSlider* instance = GUIHorizontalSlider::Create(vecoptions);
		new (B3DAllocate<ScriptGUIHorizontalSlider>())ScriptGUIHorizontalSlider(managedInstance, instance);
	}
}
