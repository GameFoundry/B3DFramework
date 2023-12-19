//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIToggleGroup.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggleGroup.h"
#include "BsScriptGUIToggleGroup.generated.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUIToggle.h"

namespace bs
{
	ScriptGUIToggleGroup::ScriptGUIToggleGroup(MonoObject* managedInstance, const SPtr<GUIToggleGroup>& value)
		:ScriptObject(managedInstance), mInternal(value)
	{
	}

	void ScriptGUIToggleGroup::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_CreateToggleGroup", (void*)&ScriptGUIToggleGroup::InternalCreateToggleGroup);

	}

	MonoObject* ScriptGUIToggleGroup::Create(const SPtr<GUIToggleGroup>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[2] = { &dummy, &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool,bool", ctorParams);
		new (B3DAllocate<ScriptGUIToggleGroup>()) ScriptGUIToggleGroup(managedInstance, value);
		return managedInstance;
	}
	void ScriptGUIToggleGroup::InternalCreateToggleGroup(MonoObject* managedInstance, bool allowAllOff)
	{
		SPtr<GUIToggleGroup> instance = GUIToggle::CreateToggleGroup(allowAllOff);
		new (B3DAllocate<ScriptGUIToggleGroup>())ScriptGUIToggleGroup(managedInstance, instance);
	}
}
