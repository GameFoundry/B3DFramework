//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIVerticalScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIVerticalScrollBar.h"
#include "BsScriptGUIVerticalScrollBar.generated.h"
#include "BsScriptGUIOption.generated.h"

namespace bs
{
	ScriptGUIVerticalScrollBar::ScriptGUIVerticalScrollBar(GUIVerticalScrollBar* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIVerticalScrollBar::~ScriptGUIVerticalScrollBar()
	{
		UnregisterEvents();
	}

	void ScriptGUIVerticalScrollBar::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIVerticalScrollBar::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIVerticalScrollBar::InternalCreate0);

	}

	MonoObject* ScriptGUIVerticalScrollBar::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIVerticalScrollBar::InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIVerticalScrollBar* nativeObject = GUIVerticalScrollBar::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIVerticalScrollBar>(nativeObject, scriptObject);
	}

	void ScriptGUIVerticalScrollBar::InternalCreate0(MonoObject* scriptObject, MonoArray* options)
	{
		TInlineArray<GUIOption, 4> nativeArrayoptions;
		if(options != nullptr)
		{
			ScriptArray scriptArrayoptions(options);
			nativeArrayoptions.resize(scriptArrayoptions.Size());
			for(int elementIndex = 0; elementIndex < (int)scriptArrayoptions.Size(); elementIndex++)
			{
				nativeArrayoptions[elementIndex] = ScriptGUIOption::FromInterop(scriptArrayoptions.Get<__GUIOptionInterop>(elementIndex));
			}
		}
		GUIVerticalScrollBar* nativeObject = GUIVerticalScrollBar::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIVerticalScrollBar>(nativeObject, scriptObject);
	}
}
