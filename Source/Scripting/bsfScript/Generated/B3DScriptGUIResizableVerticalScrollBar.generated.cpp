//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIResizableVerticalScrollBar.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIVerticalScrollBar.h"
#include "B3DScriptGUIResizableVerticalScrollBar.generated.h"
#include "B3DScriptGUIOption.generated.h"

namespace b3d
{
	ScriptGUIResizableVerticalScrollBar::ScriptGUIResizableVerticalScrollBar(GUIResizableVerticalScrollBar* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIResizableVerticalScrollBar::~ScriptGUIResizableVerticalScrollBar()
	{
		UnregisterEvents();
	}

	void ScriptGUIResizableVerticalScrollBar::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIResizableVerticalScrollBar::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIResizableVerticalScrollBar::InternalCreate0);

	}

	MonoObject* ScriptGUIResizableVerticalScrollBar::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIResizableVerticalScrollBar::InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
		GUIResizableVerticalScrollBar* nativeObject = GUIResizableVerticalScrollBar::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIResizableVerticalScrollBar>(nativeObject, scriptObject);
	}

	void ScriptGUIResizableVerticalScrollBar::InternalCreate0(MonoObject* scriptObject, MonoArray* options)
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
		GUIResizableVerticalScrollBar* nativeObject = GUIResizableVerticalScrollBar::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIResizableVerticalScrollBar>(nativeObject, scriptObject);
	}
}
