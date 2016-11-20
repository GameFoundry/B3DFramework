//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptGUIToggleGroup.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsSpriteTexture.h"
#include "BsMonoUtil.h"
#include "BsGUIToggle.h"

namespace bs
{
	ScriptGUIToggleGroup::ScriptGUIToggleGroup(MonoObject* instance, const SPtr<GUIToggleGroup>& toggleGroup)
		:ScriptObject(instance), mToggleGroup(toggleGroup)
	{

	}

	void ScriptGUIToggleGroup::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", &ScriptGUIToggleGroup::internal_createInstance);
	}

	void ScriptGUIToggleGroup::internal_createInstance(MonoObject* instance, bool allowAllOff)
	{
		SPtr<GUIToggleGroup> toggleGroup = GUIToggle::createToggleGroup(allowAllOff);

		new (bs_alloc<ScriptGUIToggleGroup>()) ScriptGUIToggleGroup(instance, toggleGroup);
	}
}