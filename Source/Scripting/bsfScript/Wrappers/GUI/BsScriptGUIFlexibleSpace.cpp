//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIFlexibleSpace.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUILayout.h"
#include "GUI/BsGUISpace.h"
#include "Wrappers/GUI/BsScriptGUILayout.h"

using namespace bs;

ScriptGUIFlexibleSpace::ScriptGUIFlexibleSpace(GUIFlexibleSpace* nativeObject)
	: TScriptGUIElementWrapper(nativeObject)
{ }

void ScriptGUIFlexibleSpace::SetupScriptBindings()
{
	sInteropMetaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIFlexibleSpace::InternalCreateInstance);
}

MonoObject* ScriptGUIFlexibleSpace::CreateScriptObject(bool construct)
{
	// TODO - Add a ctor in C# we can call if needed
	return nullptr;
}

void ScriptGUIFlexibleSpace::InternalCreateInstance(MonoObject* instance)
{
	GUIFlexibleSpace* space = GUIFlexibleSpace::Create();

	ScriptObjectWrapper::Create<ScriptGUIFlexibleSpace>(space, instance);
}
