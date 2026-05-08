//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptMotionBlurSettings.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
	ScriptMotionBlurSettings::ScriptMotionBlurSettings(const TShared<MotionBlurSettings>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptMotionBlurSettings::~ScriptMotionBlurSettings()
	{
		UnregisterEvents();
	}

	void ScriptMotionBlurSettings::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_MotionBlurSettings", (void*)&ScriptMotionBlurSettings::InternalMotionBlurSettings);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptMotionBlurSettings::InternalGetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptMotionBlurSettings::InternalSetEnabled);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetDomain", (void*)&ScriptMotionBlurSettings::InternalGetDomain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetDomain", (void*)&ScriptMotionBlurSettings::InternalSetDomain);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetFilter", (void*)&ScriptMotionBlurSettings::InternalGetFilter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetFilter", (void*)&ScriptMotionBlurSettings::InternalSetFilter);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptMotionBlurSettings::InternalGetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptMotionBlurSettings::InternalSetQuality);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetMaximumRadius", (void*)&ScriptMotionBlurSettings::InternalGetMaximumRadius);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetMaximumRadius", (void*)&ScriptMotionBlurSettings::InternalSetMaximumRadius);

	}

	MonoObject* ScriptMotionBlurSettings::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptMotionBlurSettings::InternalMotionBlurSettings(MonoObject* scriptObject)
	{
		TShared<MotionBlurSettings> nativeObject = B3DMakeShared<MotionBlurSettings>();
		ScriptObjectWrapper::Create<ScriptMotionBlurSettings>(nativeObject, scriptObject);
	}

	bool ScriptMotionBlurSettings::InternalGetEnabled(ScriptMotionBlurSettings* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MotionBlurSettings*>(self->GetNativeObject())->Enabled;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::InternalSetEnabled(ScriptMotionBlurSettings* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MotionBlurSettings*>(self->GetNativeObject())->Enabled = value;
	}

	MotionBlurDomain ScriptMotionBlurSettings::InternalGetDomain(ScriptMotionBlurSettings* self)
	{
		MotionBlurDomain tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MotionBlurSettings*>(self->GetNativeObject())->Domain;

		MotionBlurDomain __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::InternalSetDomain(ScriptMotionBlurSettings* self, MotionBlurDomain value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MotionBlurSettings*>(self->GetNativeObject())->Domain = value;
	}

	MotionBlurFilter ScriptMotionBlurSettings::InternalGetFilter(ScriptMotionBlurSettings* self)
	{
		MotionBlurFilter tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MotionBlurSettings*>(self->GetNativeObject())->Filter;

		MotionBlurFilter __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::InternalSetFilter(ScriptMotionBlurSettings* self, MotionBlurFilter value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MotionBlurSettings*>(self->GetNativeObject())->Filter = value;
	}

	MotionBlurQuality ScriptMotionBlurSettings::InternalGetQuality(ScriptMotionBlurSettings* self)
	{
		MotionBlurQuality tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MotionBlurSettings*>(self->GetNativeObject())->Quality;

		MotionBlurQuality __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::InternalSetQuality(ScriptMotionBlurSettings* self, MotionBlurQuality value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MotionBlurSettings*>(self->GetNativeObject())->Quality = value;
	}

	float ScriptMotionBlurSettings::InternalGetMaximumRadius(ScriptMotionBlurSettings* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<MotionBlurSettings*>(self->GetNativeObject())->MaximumRadius;

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptMotionBlurSettings::InternalSetMaximumRadius(ScriptMotionBlurSettings* self, float value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<MotionBlurSettings*>(self->GetNativeObject())->MaximumRadius = value;
	}
}
