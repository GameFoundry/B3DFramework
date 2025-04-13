//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUILayoutX.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUILayoutX.h"
#include "BsScriptGUILayoutX.generated.h"
#include "BsScriptGUIOption.generated.h"

namespace bs
{
	ScriptGUILayoutX::ScriptGUILayoutX(GUILayoutX* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	void ScriptGUILayoutX::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUILayoutX::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUILayoutX::InternalCreate0);

	}

	MonoObject* ScriptGUILayoutX::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUILayoutX::InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
		GUILayoutX* nativeObject = GUILayoutX::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUILayoutX>(nativeObject, scriptObject);
	}

	void ScriptGUILayoutX::InternalCreate0(MonoObject* scriptObject, MonoArray* options)
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
		GUILayoutX* nativeObject = GUILayoutX::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUILayoutX>(nativeObject, scriptObject);
	}
}
