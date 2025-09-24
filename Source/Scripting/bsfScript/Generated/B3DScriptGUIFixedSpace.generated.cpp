//********************************* B3D Framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptGUIFixedSpace.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Foundation/Engine/GUI/B3DGUISpace.h"
#include "B3DScriptTUnitValue.generated.h"
#include "B3DScriptGUIFixedSpace.generated.h"

namespace b3d
{
	ScriptGUIFixedSpace::ScriptGUIFixedSpace(GUIFixedSpace* nativeObject)
		:TScriptGUIElementWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptGUIFixedSpace::~ScriptGUIFixedSpace()
	{
		UnregisterEvents();
	}

	void ScriptGUIFixedSpace::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetSize", (void*)&ScriptGUIFixedSpace::InternalGetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetSize", (void*)&ScriptGUIFixedSpace::InternalSetSize);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Create", (void*)&ScriptGUIFixedSpace::InternalCreate);

	}

	MonoObject* ScriptGUIFixedSpace::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptGUIFixedSpace::InternalGetSize(ScriptGUIFixedSpace* self, TUnitValue<int32_t, LogicalPixel>* __output)
	{
		if(!self->IsNativeObjectValid())
		{
			*__output = {};
			return;
		}

		TUnitValue<int32_t, LogicalPixel> tmp__output;
		tmp__output = static_cast<GUIFixedSpace*>(self->GetNativeObject())->GetSize();

		*__output = tmp__output;
	}

	void ScriptGUIFixedSpace::InternalSetSize(ScriptGUIFixedSpace* self, TUnitValue<int32_t, LogicalPixel>* size)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<GUIFixedSpace*>(self->GetNativeObject())->SetSize(*size);
	}

	void ScriptGUIFixedSpace::InternalCreate(MonoObject* scriptObject, TUnitValue<int32_t, LogicalPixel>* size)
	{
		GUIFixedSpace* nativeObject = GUIFixedSpace::Create(*size);
		ScriptObjectWrapper::Create<ScriptGUIFixedSpace>(nativeObject, scriptObject);
	}
}
