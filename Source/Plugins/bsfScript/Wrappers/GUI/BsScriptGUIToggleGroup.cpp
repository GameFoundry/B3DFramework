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

namespace bs
{
	ScriptGUIToggleGroup::ScriptGUIToggleGroup(MonoObject* instance, const SPtr<GUIToggleGroup>& toggleGroup)
		:ScriptObject(instance), mToggleGroup(toggleGroup)
	{

	}

	void ScriptGUIToggleGroup::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_CreateInstance", (void*)&ScriptGUIToggleGroup::internal_createInstance);
	}

	void ScriptGUIToggleGroup::internal_createInstance(MonoObject* instance, bool allowAllOff)
	{
		SPtr<GUIToggleGroup> toggleGroup = GUIToggle::createToggleGroup(allowAllOff);

		new (bs_alloc<ScriptGUIToggleGroup>()) ScriptGUIToggleGroup(instance, toggleGroup);
	}
}