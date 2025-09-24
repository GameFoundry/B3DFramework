//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIToggleGroup.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUIToggleGroup.h"
#include "B3DScriptGUIToggleGroup.generated.h"

namespace b3d
{
	ScriptGUIToggleGroup::ScriptGUIToggleGroup(const SPtr<GUIToggleGroup>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIToggleGroup::~ScriptGUIToggleGroup()
	{
		UnregisterEvents();
	}

	void ScriptGUIToggleGroup::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIToggleGroup::InternalCreate);

	}

	MonoObject* ScriptGUIToggleGroup::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[2] = { &dummy, &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool,bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIToggleGroup::InternalCreate(MonoObject* scriptObject, bool allowAllOff)
	{
		SPtr<GUIToggleGroup> nativeObject = GUIToggleGroup::Create(allowAllOff);
		ScriptObjectWrapper::Create<ScriptGUIToggleGroup>(nativeObject, scriptObject);
	}
}
