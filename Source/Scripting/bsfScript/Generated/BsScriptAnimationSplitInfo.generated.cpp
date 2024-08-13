//********************************* bs::framework - Copyright 2018-2022 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationSplitInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
#if !B3D_IS_ENGINE
	ScriptAnimationSplitInfo::ScriptAnimationSplitInfo(MonoObject* managedInstance, const SPtr<AnimationSplitInfo>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptAnimationSplitInfo::InitRuntimeData()
	{
		metaData.ScriptClass->AddInternalCall("Internal_AnimationSplitInfo", (void*)&ScriptAnimationSplitInfo::InternalAnimationSplitInfo);
		metaData.ScriptClass->AddInternalCall("Internal_AnimationSplitInfo0", (void*)&ScriptAnimationSplitInfo::InternalAnimationSplitInfo0);
		metaData.ScriptClass->AddInternalCall("Internal_GetName", (void*)&ScriptAnimationSplitInfo::InternalGetName);
		metaData.ScriptClass->AddInternalCall("Internal_SetName", (void*)&ScriptAnimationSplitInfo::InternalSetName);
		metaData.ScriptClass->AddInternalCall("Internal_GetStartFrame", (void*)&ScriptAnimationSplitInfo::InternalGetStartFrame);
		metaData.ScriptClass->AddInternalCall("Internal_SetStartFrame", (void*)&ScriptAnimationSplitInfo::InternalSetStartFrame);
		metaData.ScriptClass->AddInternalCall("Internal_GetEndFrame", (void*)&ScriptAnimationSplitInfo::InternalGetEndFrame);
		metaData.ScriptClass->AddInternalCall("Internal_SetEndFrame", (void*)&ScriptAnimationSplitInfo::InternalSetEndFrame);
		metaData.ScriptClass->AddInternalCall("Internal_GetIsAdditive", (void*)&ScriptAnimationSplitInfo::InternalGetIsAdditive);
		metaData.ScriptClass->AddInternalCall("Internal_SetIsAdditive", (void*)&ScriptAnimationSplitInfo::InternalSetIsAdditive);

	}

	MonoObject* ScriptAnimationSplitInfo::Create(const SPtr<AnimationSplitInfo>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.ScriptClass->CreateInstance("bool", ctorParams);
		new (B3DAllocate<ScriptAnimationSplitInfo>()) ScriptAnimationSplitInfo(managedInstance, value);
		return managedInstance;
	}
	void ScriptAnimationSplitInfo::InternalAnimationSplitInfo(MonoObject* managedInstance)
	{
		SPtr<AnimationSplitInfo> nativeObject = B3DMakeShared<AnimationSplitInfo>();
		new (B3DAllocate<ScriptAnimationSplitInfo>())ScriptAnimationSplitInfo(managedInstance, nativeObject);
	}

	void ScriptAnimationSplitInfo::InternalAnimationSplitInfo0(MonoObject* managedInstance, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive)
	{
		String tmpname;
		tmpname = MonoUtil::MonoToString(name);
		SPtr<AnimationSplitInfo> nativeObject = B3DMakeShared<AnimationSplitInfo>(tmpname, startFrame, endFrame, isAdditive);
		new (B3DAllocate<ScriptAnimationSplitInfo>())ScriptAnimationSplitInfo(managedInstance, nativeObject);
	}

	MonoString* ScriptAnimationSplitInfo::InternalGetName(ScriptAnimationSplitInfo* self)
	{
		String tmp__output;
		tmp__output = self->GetInternal()->Name;

		MonoString* __output;
		__output = MonoUtil::StringToMono(tmp__output);

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetName(ScriptAnimationSplitInfo* self, MonoString* value)
	{
		String tmpvalue;
		tmpvalue = MonoUtil::MonoToString(value);
		self->GetInternal()->Name = tmpvalue;
	}

	uint32_t ScriptAnimationSplitInfo::InternalGetStartFrame(ScriptAnimationSplitInfo* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->StartFrame;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetStartFrame(ScriptAnimationSplitInfo* self, uint32_t value)
	{
		self->GetInternal()->StartFrame = value;
	}

	uint32_t ScriptAnimationSplitInfo::InternalGetEndFrame(ScriptAnimationSplitInfo* self)
	{
		uint32_t tmp__output;
		tmp__output = self->GetInternal()->EndFrame;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetEndFrame(ScriptAnimationSplitInfo* self, uint32_t value)
	{
		self->GetInternal()->EndFrame = value;
	}

	bool ScriptAnimationSplitInfo::InternalGetIsAdditive(ScriptAnimationSplitInfo* self)
	{
		bool tmp__output;
		tmp__output = self->GetInternal()->IsAdditive;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::InternalSetIsAdditive(ScriptAnimationSplitInfo* self, bool value)
	{
		self->GetInternal()->IsAdditive = value;
	}
#endif
}
