//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIResizableHorizontalScrollBar.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIHorizontalScrollBar.h"
#include "BsScriptGUIResizableHorizontalScrollBar.generated.h"
#include "BsScriptGUIOption.generated.h"

namespace bs
{
	ScriptGUIResizableHorizontalScrollBar::ScriptGUIResizableHorizontalScrollBar(GUIResizableHorizontalScrollBar* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIResizableHorizontalScrollBar::~ScriptGUIResizableHorizontalScrollBar()
	{
		UnregisterEvents();
	}

	void ScriptGUIResizableHorizontalScrollBar::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIResizableHorizontalScrollBar::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIResizableHorizontalScrollBar::InternalCreate0);

	}

	MonoObject* ScriptGUIResizableHorizontalScrollBar::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIResizableHorizontalScrollBar::InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
		GUIResizableHorizontalScrollBar* nativeObject = GUIResizableHorizontalScrollBar::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIResizableHorizontalScrollBar>(nativeObject, scriptObject);
	}

	void ScriptGUIResizableHorizontalScrollBar::InternalCreate0(MonoObject* scriptObject, MonoArray* options)
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
		GUIResizableHorizontalScrollBar* nativeObject = GUIResizableHorizontalScrollBar::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIResizableHorizontalScrollBar>(nativeObject, scriptObject);
	}
}
