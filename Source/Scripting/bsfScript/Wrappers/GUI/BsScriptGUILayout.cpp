//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUILayout.h"
#include "BsMonoClass.h"
#include "GUI/BsGUIScrollArea.h"
#include "BsMonoUtil.h"

using namespace bs;

ScriptGUIPanel::ScriptGUIPanel(GUIPanel* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{}

void ScriptGUIPanel::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIPanel::InternalCreate);
}

MonoObject* ScriptGUIPanel::CreateScriptObject(bool construct)
{
	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	if(construct)
		return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

	return sInteropMetaData.ScriptClass->CreateInstance(false);
}

void ScriptGUIPanel::InternalCreate(MonoObject* instance, i16 depth, u16 depthRangeMin, u32 depthRangeMax, MonoArray* guiOptions)
{
	GUIOptions options;

	ScriptArray scriptArray(guiOptions);
	u32 arrayLen = scriptArray.Size();
	for(u32 i = 0; i < arrayLen; i++)
		options.AddOption(scriptArray.Get<GUIOption>(i));

	GUIPanel* panel = GUIPanel::Create(depth, depthRangeMin, depthRangeMax, options);
	ScriptObjectWrapper::Create<ScriptGUIPanel>(panel, instance);
}
