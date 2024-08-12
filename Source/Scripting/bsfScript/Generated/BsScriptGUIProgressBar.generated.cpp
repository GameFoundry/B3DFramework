//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIProgressBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIProgressBar.h"
#include "BsScriptGUIOption.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIProgressBar.h"

namespace bs
{
	ScriptGUIProgressBar::ScriptGUIProgressBar(MonoObject* managedInstance, GUIProgressBar* value)
		:TScriptGUIInteractable(managedInstance, value)
	{
		RegisterEvents(value);
	}

	void ScriptGUIProgressBar::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUIProgressBar::InternalSetPercent);
		metaData.ScriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUIProgressBar::InternalGetPercent);
		metaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIProgressBar::InternalCreate);
		metaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIProgressBar::InternalCreate0);

	}

	void ScriptGUIProgressBar::InternalSetPercent(ScriptGUIProgressBar* thisPtr, float percent)
	{
		static_cast<GUIProgressBar*>(thisPtr->GetGuiElement())->SetPercent(percent);
	}

	float ScriptGUIProgressBar::InternalGetPercent(ScriptGUIProgressBar* thisPtr)
	{
		float tmp__output;
		tmp__output = static_cast<GUIProgressBar*>(thisPtr->GetGuiElement())->GetPercent();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIProgressBar::InternalCreate(MonoObject* managedInstance, MonoString* styleClass, MonoArray* options)
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
		GUIProgressBar* instance = GUIProgressBar::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIProgressBar>())ScriptGUIProgressBar(managedInstance, instance);
	}

	void ScriptGUIProgressBar::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIProgressBar* instance = GUIProgressBar::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUIProgressBar>())ScriptGUIProgressBar(managedInstance, instance);
	}
}
