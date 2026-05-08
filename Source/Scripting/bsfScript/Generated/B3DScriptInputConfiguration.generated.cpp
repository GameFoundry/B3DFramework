//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptInputConfiguration.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Core/Input/B3DInputConfiguration.h"
#include "B3DScriptVirtualAxisCreateInformation.generated.h"

namespace b3d
{
	ScriptInputConfiguration::ScriptInputConfiguration(const TShared<InputConfiguration>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptInputConfiguration::~ScriptInputConfiguration()
	{
		UnregisterEvents();
	}

	void ScriptInputConfiguration::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_InputConfiguration", (void*)&ScriptInputConfiguration::InternalInputConfiguration);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RegisterButton", (void*)&ScriptInputConfiguration::InternalRegisterButton);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UnregisterButton", (void*)&ScriptInputConfiguration::InternalUnregisterButton);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_RegisterAxis", (void*)&ScriptInputConfiguration::InternalRegisterAxis);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_UnregisterAxis", (void*)&ScriptInputConfiguration::InternalUnregisterAxis);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetRepeatInterval", (void*)&ScriptInputConfiguration::InternalSetRepeatInterval);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetRepeatInterval", (void*)&ScriptInputConfiguration::InternalGetRepeatInterval);

	}

	MonoObject* ScriptInputConfiguration::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptInputConfiguration::InternalInputConfiguration(MonoObject* scriptObject)
	{
		TShared<InputConfiguration> nativeObject = B3DMakeShared<InputConfiguration>();
		ScriptObjectWrapper::Create<ScriptInputConfiguration>(nativeObject, scriptObject);
	}

	void ScriptInputConfiguration::InternalRegisterButton(ScriptInputConfiguration* self, MonoString* name, ButtonCode buttonCode, ButtonModifier modifiers, bool repeatable)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<InputConfiguration*>(self->GetNativeObject())->RegisterButton(tmpname, buttonCode, modifiers, repeatable);
	}

	void ScriptInputConfiguration::InternalUnregisterButton(ScriptInputConfiguration* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<InputConfiguration*>(self->GetNativeObject())->UnregisterButton(tmpname);
	}

	void ScriptInputConfiguration::InternalRegisterAxis(ScriptInputConfiguration* self, MonoString* name, VirtualAxisCreateInformation* createInformation)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<InputConfiguration*>(self->GetNativeObject())->RegisterAxis(tmpname, *createInformation);
	}

	void ScriptInputConfiguration::InternalUnregisterAxis(ScriptInputConfiguration* self, MonoString* name)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		static_cast<InputConfiguration*>(self->GetNativeObject())->UnregisterAxis(tmpname);
	}

	void ScriptInputConfiguration::InternalSetRepeatInterval(ScriptInputConfiguration* self, uint64_t milliseconds)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<InputConfiguration*>(self->GetNativeObject())->SetRepeatInterval(milliseconds);
	}

	uint64_t ScriptInputConfiguration::InternalGetRepeatInterval(ScriptInputConfiguration* self)
	{
		uint64_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<InputConfiguration*>(self->GetNativeObject())->GetRepeatInterval();

		uint64_t __output;
		__output = tmp__output;

		return __output;
	}
}
