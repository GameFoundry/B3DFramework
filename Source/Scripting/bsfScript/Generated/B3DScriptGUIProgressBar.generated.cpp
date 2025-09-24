//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIProgressBar.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIProgressBar.h"
#include "B3DScriptGUIProgressBar.generated.h"
#include "B3DScriptGUIOption.generated.h"

namespace b3d
{
	ScriptGUIProgressBar::ScriptGUIProgressBar(GUIProgressBar* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIProgressBar::~ScriptGUIProgressBar()
	{
		UnregisterEvents();
	}

	void ScriptGUIProgressBar::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPercent", (void*)&ScriptGUIProgressBar::InternalSetPercent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetPercent", (void*)&ScriptGUIProgressBar::InternalGetPercent);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIProgressBar::InternalCreate);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create0", (void*)&ScriptGUIProgressBar::InternalCreate0);

	}

	MonoObject* ScriptGUIProgressBar::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIProgressBar::InternalSetPercent(ScriptGUIProgressBar* self, float percent)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIProgressBar*>(self->GetNativeObject())->SetPercent(percent);
	}

	float ScriptGUIProgressBar::InternalGetPercent(ScriptGUIProgressBar* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<GUIProgressBar*>(self->GetNativeObject())->GetPercent();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptGUIProgressBar::InternalCreate(MonoObject* scriptObject, MonoString* styleClass, MonoArray* options)
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
		GUIProgressBar* nativeObject = GUIProgressBar::Create(tmpstyleClass, nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIProgressBar>(nativeObject, scriptObject);
	}

	void ScriptGUIProgressBar::InternalCreate0(MonoObject* scriptObject, MonoArray* options)
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
		GUIProgressBar* nativeObject = GUIProgressBar::Create(nativeArrayoptions);
		ScriptObjectWrapper::Create<ScriptGUIProgressBar>(nativeObject, scriptObject);
	}
}
