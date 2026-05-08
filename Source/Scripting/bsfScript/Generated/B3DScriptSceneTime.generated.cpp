//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptSceneTime.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"
#include "../../../Engine/Utility/Utility/B3DTime.h"

namespace b3d
{
	ScriptSceneTime::ScriptSceneTime(const TShared<SceneTime>& nativeObject)
		:TScriptNonReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptSceneTime::~ScriptSceneTime()
	{
		UnregisterEvents();
	}

	void ScriptSceneTime::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetTimeInSeconds", (void*)&ScriptSceneTime::InternalGetTimeInSeconds);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetScale", (void*)&ScriptSceneTime::InternalSetScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetScale", (void*)&ScriptSceneTime::InternalGetScale);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_Reset", (void*)&ScriptSceneTime::InternalReset);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetPaused", (void*)&ScriptSceneTime::InternalSetPaused);

	}

	MonoObject* ScriptSceneTime::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	float ScriptSceneTime::InternalGetTimeInSeconds(ScriptSceneTime* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneTime*>(self->GetNativeObject())->GetTimeInSeconds();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSceneTime::InternalSetScale(ScriptSceneTime* self, float scale)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SceneTime*>(self->GetNativeObject())->SetScale(scale);
	}

	float ScriptSceneTime::InternalGetScale(ScriptSceneTime* self)
	{
		float tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<SceneTime*>(self->GetNativeObject())->GetScale();

		float __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptSceneTime::InternalReset(ScriptSceneTime* self)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SceneTime*>(self->GetNativeObject())->Reset();
	}

	void ScriptSceneTime::InternalSetPaused(ScriptSceneTime* self, bool paused)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<SceneTime*>(self->GetNativeObject())->SetPaused(paused);
	}
}
