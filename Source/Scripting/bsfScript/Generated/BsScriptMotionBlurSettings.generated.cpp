//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptMotionBlurSettings.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

using namespace bs;
ScriptMotionBlurSettings::ScriptMotionBlurSettings(MonoObject* managedInstance, const SPtr<MotionBlurSettings>& value)
	: TScriptReflectable(managedInstance, value)
{
}

void ScriptMotionBlurSettings::InitRuntimeData()
{
	metaData.ScriptClass->AddInternalCall("Internal_MotionBlurSettings", (void*)&ScriptMotionBlurSettings::InternalMotionBlurSettings);
	metaData.ScriptClass->AddInternalCall("Internal_GetEnabled", (void*)&ScriptMotionBlurSettings::InternalGetEnabled);
	metaData.ScriptClass->AddInternalCall("Internal_SetEnabled", (void*)&ScriptMotionBlurSettings::InternalSetEnabled);
	metaData.ScriptClass->AddInternalCall("Internal_GetDomain", (void*)&ScriptMotionBlurSettings::InternalGetDomain);
	metaData.ScriptClass->AddInternalCall("Internal_SetDomain", (void*)&ScriptMotionBlurSettings::InternalSetDomain);
	metaData.ScriptClass->AddInternalCall("Internal_GetFilter", (void*)&ScriptMotionBlurSettings::InternalGetFilter);
	metaData.ScriptClass->AddInternalCall("Internal_SetFilter", (void*)&ScriptMotionBlurSettings::InternalSetFilter);
	metaData.ScriptClass->AddInternalCall("Internal_GetQuality", (void*)&ScriptMotionBlurSettings::InternalGetQuality);
	metaData.ScriptClass->AddInternalCall("Internal_SetQuality", (void*)&ScriptMotionBlurSettings::InternalSetQuality);
	metaData.ScriptClass->AddInternalCall("Internal_GetMaximumRadius", (void*)&ScriptMotionBlurSettings::InternalGetMaximumRadius);
	metaData.ScriptClass->AddInternalCall("Internal_SetMaximumRadius", (void*)&ScriptMotionBlurSettings::InternalSetMaximumRadius);
}

MonoObject* ScriptMotionBlurSettings::Create(const SPtr<MotionBlurSettings>& value)
{
	if(value == nullptr) return nullptr;

	bool dummy = false;
	void* ctorParams[1] = { &dummy };

	MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
	new(B3DAllocate<ScriptMotionBlurSettings>()) ScriptMotionBlurSettings(managedInstance, value);
	return managedInstance;
}

void ScriptMotionBlurSettings::InternalMotionBlurSettings(MonoObject* managedInstance)
{
	SPtr<MotionBlurSettings> instance = B3DMakeShared<MotionBlurSettings>();
	new(B3DAllocate<ScriptMotionBlurSettings>()) ScriptMotionBlurSettings(managedInstance, instance);
}

bool ScriptMotionBlurSettings::InternalGetEnabled(ScriptMotionBlurSettings* thisPtr)
{
	bool tmp__output;
	tmp__output = thisPtr->GetInternal()->Enabled;

	bool __output;
	__output = tmp__output;

	return __output;
}

void ScriptMotionBlurSettings::InternalSetEnabled(ScriptMotionBlurSettings* thisPtr, bool value)
{
	thisPtr->GetInternal()->Enabled = value;
}

MotionBlurDomain ScriptMotionBlurSettings::InternalGetDomain(ScriptMotionBlurSettings* thisPtr)
{
	MotionBlurDomain tmp__output;
	tmp__output = thisPtr->GetInternal()->Domain;

	MotionBlurDomain __output;
	__output = tmp__output;

	return __output;
}

void ScriptMotionBlurSettings::InternalSetDomain(ScriptMotionBlurSettings* thisPtr, MotionBlurDomain value)
{
	thisPtr->GetInternal()->Domain = value;
}

MotionBlurFilter ScriptMotionBlurSettings::InternalGetFilter(ScriptMotionBlurSettings* thisPtr)
{
	MotionBlurFilter tmp__output;
	tmp__output = thisPtr->GetInternal()->Filter;

	MotionBlurFilter __output;
	__output = tmp__output;

	return __output;
}

void ScriptMotionBlurSettings::InternalSetFilter(ScriptMotionBlurSettings* thisPtr, MotionBlurFilter value)
{
	thisPtr->GetInternal()->Filter = value;
}

MotionBlurQuality ScriptMotionBlurSettings::InternalGetQuality(ScriptMotionBlurSettings* thisPtr)
{
	MotionBlurQuality tmp__output;
	tmp__output = thisPtr->GetInternal()->Quality;

	MotionBlurQuality __output;
	__output = tmp__output;

	return __output;
}

void ScriptMotionBlurSettings::InternalSetQuality(ScriptMotionBlurSettings* thisPtr, MotionBlurQuality value)
{
	thisPtr->GetInternal()->Quality = value;
}

float ScriptMotionBlurSettings::InternalGetMaximumRadius(ScriptMotionBlurSettings* thisPtr)
{
	float tmp__output;
	tmp__output = thisPtr->GetInternal()->MaximumRadius;

	float __output;
	__output = tmp__output;

	return __output;
}

void ScriptMotionBlurSettings::InternalSetMaximumRadius(ScriptMotionBlurSettings* thisPtr, float value)
{
	thisPtr->GetInternal()->MaximumRadius = value;
}
