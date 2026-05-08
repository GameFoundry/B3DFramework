//************************************ B3D Framework - Copyright 2025 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "B3DScriptAnimationSplitInfo.generated.h"
#include "B3DMonoMethod.h"
#include "B3DMonoClass.h"
#include "B3DMonoUtil.h"

namespace b3d
{
#if !B3D_IS_ENGINE
	ScriptAnimationSplitInfo::ScriptAnimationSplitInfo(const TShared<AnimationSplitInfo>& nativeObject)
		:TScriptReflectableWrapper(nativeObject)
	{
		RegisterEvents();
	}

	ScriptAnimationSplitInfo::~ScriptAnimationSplitInfo()
	{
		UnregisterEvents();
	}

	void ScriptAnimationSplitInfo::SetupScriptBindings()
	{
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AnimationSplitInfo", (void*)&ScriptAnimationSplitInfo::InternalAnimationSplitInfo);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_AnimationSplitInfo0", (void*)&ScriptAnimationSplitInfo::InternalAnimationSplitInfo0);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptAnimationSplitInfo::InternalGetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptAnimationSplitInfo::InternalSetName);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetStartFrame", (void*)&ScriptAnimationSplitInfo::InternalGetStartFrame);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetStartFrame", (void*)&ScriptAnimationSplitInfo::InternalSetStartFrame);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetEndFrame", (void*)&ScriptAnimationSplitInfo::InternalGetEndFrame);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetEndFrame", (void*)&ScriptAnimationSplitInfo::InternalSetEndFrame);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_GetIsAdditive", (void*)&ScriptAnimationSplitInfo::InternalGetIsAdditive);
		sInteropMetaData.ScriptClass->AddInternalCall("Internal_SetIsAdditive", (void*)&ScriptAnimationSplitInfo::InternalSetIsAdditive);

	}

	MonoObject* ScriptAnimationSplitInfo::CreateScriptObject(bool construct)
	{
		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		if(construct)
			return sInteropMetaData.ScriptClass->CreateInstance("bool", ctorParams);

		return sInteropMetaData.ScriptClass->CreateInstance(false);
	}
	void ScriptAnimationSplitInfo::InternalAnimationSplitInfo(MonoObject* scriptObject)
	{
		TShared<AnimationSplitInfo> nativeObject = B3DMakeShared<AnimationSplitInfo>();
		ScriptObjectWrapper::Create<ScriptAnimationSplitInfo>(nativeObject, scriptObject);
	}

	void ScriptAnimationSplitInfo::InternalAnimationSplitInfo0(MonoObject* scriptObject, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		TShared<AnimationSplitInfo> nativeObject = B3DMakeShared<AnimationSplitInfo>(tmpname, startFrame, endFrame, isAdditive);
		ScriptObjectWrapper::Create<ScriptAnimationSplitInfo>(nativeObject, scriptObject);
	}

	MonoString* ScriptAnimationSplitInfo::InternalGetName(ScriptAnimationSplitInfo* self)
	{
		String tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationSplitInfo*>(self->GetNativeObject())->Name;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetName(ScriptAnimationSplitInfo* self, MonoString* value)
	{
		if(!self->IsNativeObjectValid())
			return;

		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		static_cast<AnimationSplitInfo*>(self->GetNativeObject())->Name = tmpvalue;
	}

	uint32_t ScriptAnimationSplitInfo::InternalGetStartFrame(ScriptAnimationSplitInfo* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationSplitInfo*>(self->GetNativeObject())->StartFrame;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetStartFrame(ScriptAnimationSplitInfo* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AnimationSplitInfo*>(self->GetNativeObject())->StartFrame = value;
	}

	uint32_t ScriptAnimationSplitInfo::InternalGetEndFrame(ScriptAnimationSplitInfo* self)
	{
		uint32_t tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationSplitInfo*>(self->GetNativeObject())->EndFrame;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetEndFrame(ScriptAnimationSplitInfo* self, uint32_t value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AnimationSplitInfo*>(self->GetNativeObject())->EndFrame = value;
	}

	bool ScriptAnimationSplitInfo::InternalGetIsAdditive(ScriptAnimationSplitInfo* self)
	{
		bool tmp__output;
		if(!self->IsNativeObjectValid())
			return {};

		tmp__output = static_cast<AnimationSplitInfo*>(self->GetNativeObject())->IsAdditive;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetIsAdditive(ScriptAnimationSplitInfo* self, bool value)
	{
		if(!self->IsNativeObjectValid())
			return;

		static_cast<AnimationSplitInfo*>(self->GetNativeObject())->IsAdditive = value;
	}
#endif
}
