//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptGUIFlexibleSpace.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"
#include "../../../Foundation/bsfEngine/GUI/BsGUISpace.h"
#include "BsScriptGUIFlexibleSpace.generated.h"

namespace bs
{
	ScriptGUIFlexibleSpace::ScriptGUIFlexibleSpace(GUIFlexibleSpace* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIFlexibleSpace::~ScriptGUIFlexibleSpace()
	{
		UnregisterEvents();
	}

	void ScriptGUIFlexibleSpace::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIFlexibleSpace::InternalCreate);

	}

	MonoObject* ScriptGUIFlexibleSpace::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIFlexibleSpace::InternalCreate(MonoObject* scriptObject)
	{
		GUIFlexibleSpace* nativeObject = GUIFlexibleSpace::Create();
		ScriptObjectWrapper::Create<ScriptGUIFlexibleSpace>(nativeObject, scriptObject);
	}
}
