//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggle.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggle.h"
#include "BsScriptGUIToggle.generated.h"
#include "BsScriptGUIOption.generated.h"
#include "BsScriptGUIToggleContent.generated.h"

namespace bs
{
	ScriptGUIToggle::ScriptGUIToggle(GUIToggle* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIToggle::~ScriptGUIToggle()
	{
		UnregisterEvents();
	}

	void ScriptGUIToggle::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIToggle::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIToggle::InternalCreate0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create1", (void*)&ScriptGUIToggle::InternalCreate1);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create2", (void*)&ScriptGUIToggle::InternalCreate2);

	}

	MonoObject* ScriptGUIToggle::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIToggle::InternalCreate(MonoObject* scriptObject, __GUIToggleContentInterop* contents, MonoString* styleClass, MonoArray* options)
	{
		GUIToggleContent tmpcontents;
		tmpcontents = ScriptGUIToggleContent::FromInterop(*contents);
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
		GUIToggle* nativeObject = GUIToggle::Create(tmpcontents, tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIToggle>(nativeObject, scriptObject);
	}

	void ScriptGUIToggle::InternalCreate0(MonoObject* scriptObject, __GUIToggleContentInterop* contents, MonoArray* options)
	{
		GUIToggleContent tmpcontents;
		tmpcontents = ScriptGUIToggleContent::FromInterop(*contents);
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
		GUIToggle* nativeObject = GUIToggle::Create(tmpcontents, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIToggle>(nativeObject, scriptObject);
	}

	void ScriptGUIToggle::InternalCreate1(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
		GUIToggle* nativeObject = GUIToggle::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIToggle>(nativeObject, scriptObject);
	}

	void ScriptGUIToggle::InternalCreate2(MonoObject* scriptObject, MonoArray* options)
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
		GUIToggle* nativeObject = GUIToggle::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIToggle>(nativeObject, scriptObject);
	}
}
