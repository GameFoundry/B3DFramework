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
		GUIResizableHorizontalScrollBar* nativeObject = GUIResizableHorizontalScrollBar::Create(tmpstyleClass, nativeArrayoptions);
		new (B3DAllocate<ScriptGUIResizableHorizontalScrollBar>())ScriptGUIResizableHorizontalScrollBar(managedInstance, nativeObject);
	}

	void ScriptGUIResizableHorizontalScrollBar::InternalCreate0(MonoObject* managedInstance, MonoArray* options)
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
		GUIResizableHorizontalScrollBar* nativeObject = GUIResizableHorizontalScrollBar::Create(nativeArrayoptions);
		new (B3DAllocate<ScriptGUIResizableHorizontalScrollBar>())ScriptGUIResizableHorizontalScrollBar(managedInstance, nativeObject);
	}
}
