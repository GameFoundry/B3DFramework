//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Wrappers/GUI/BsScriptGUIToggleGroup.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "Image/BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "GUI/BsGUIToggle.h"

using namespace bs;
ScriptGUIToggleGroup::ScriptGUIToggleGroup(MonoObject* instance, const SPtr<GUIToggleGroup>& toggleGroup)
	: ScriptObject(instance), mToggleGroup(toggleGroup)
{
}

void ScriptGUIToggleGroup::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_CreateInstance", (void*)&ScriptGUIToggleGroup::InternalCreateInstance);
}

void ScriptGUIToggleGroup::InternalCreateInstance(MonoObject* instance, bool allowAllOff)
{
	SPtr<GUIToggleGroup> toggleGroup = GUIToggle::CreateToggleGroup(allowAllOff);

	new(B3DAllocate<ScriptGUIToggleGroup>()) ScriptGUIToggleGroup(instance, toggleGroup);
}
