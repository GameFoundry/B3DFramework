//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsScriptAnimationSplitInfo.generated.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoUtil.h"

namespace bs
{
#if !BS_IS_BANSHEE3D
	ScriptAnimationSplitInfo::ScriptAnimationSplitInfo(MonoObject* managedInstance, const SPtr<AnimationSplitInfo>& value)
		:TScriptReflectable(managedInstance, value)
	{
	}

	void ScriptAnimationSplitInfo::InitRuntimeData()
	{
		metaData.scriptClass->AddInternalCall("Internal_AnimationSplitInfo", (void*)&ScriptAnimationSplitInfo::Internal_AnimationSplitInfo);
		metaData.scriptClass->AddInternalCall("Internal_AnimationSplitInfo0", (void*)&ScriptAnimationSplitInfo::Internal_AnimationSplitInfo0);
		metaData.scriptClass->AddInternalCall("Internal_getname", (void*)&ScriptAnimationSplitInfo::Internal_getname);
		metaData.scriptClass->AddInternalCall("Internal_setname", (void*)&ScriptAnimationSplitInfo::Internal_setname);
		metaData.scriptClass->AddInternalCall("Internal_getstartFrame", (void*)&ScriptAnimationSplitInfo::Internal_getstartFrame);
		metaData.scriptClass->AddInternalCall("Internal_setstartFrame", (void*)&ScriptAnimationSplitInfo::Internal_setstartFrame);
		metaData.scriptClass->AddInternalCall("Internal_getendFrame", (void*)&ScriptAnimationSplitInfo::Internal_getendFrame);
		metaData.scriptClass->AddInternalCall("Internal_setendFrame", (void*)&ScriptAnimationSplitInfo::Internal_setendFrame);
		metaData.scriptClass->AddInternalCall("Internal_getisAdditive", (void*)&ScriptAnimationSplitInfo::Internal_getisAdditive);
		metaData.scriptClass->AddInternalCall("Internal_setisAdditive", (void*)&ScriptAnimationSplitInfo::Internal_setisAdditive);

	}

	MonoObject* ScriptAnimationSplitInfo::create(const SPtr<AnimationSplitInfo>& value)
	{
		if(value == nullptr) return nullptr; 

		bool dummy = false;
		void* ctorParams[1] = { &dummy };

		MonoObject* managedInstance = metaData.scriptClass->CreateInstance("bool", ctorParams);
		new (bs_alloc<ScriptAnimationSplitInfo>()) ScriptAnimationSplitInfo(managedInstance, value);
		return managedInstance;
	}
	void ScriptAnimationSplitInfo::Internal_AnimationSplitInfo(MonoObject* managedInstance)
	{
		SPtr<AnimationSplitInfo> instance = bs_shared_ptr_new<AnimationSplitInfo>();
		new (bs_alloc<ScriptAnimationSplitInfo>())ScriptAnimationSplitInfo(managedInstance, instance);
	}

	void ScriptAnimationSplitInfo::Internal_AnimationSplitInfo0(MonoObject* managedInstance, MonoString* name, uint32_t startFrame, uint32_t endFrame, bool isAdditive)
	{
		String tmpname;
		tmpname = MonoUtil::monoToString(name);
		SPtr<AnimationSplitInfo> instance = bs_shared_ptr_new<AnimationSplitInfo>(tmpname, startFrame, endFrame, isAdditive);
		new (bs_alloc<ScriptAnimationSplitInfo>())ScriptAnimationSplitInfo(managedInstance, instance);
	}

	MonoString* ScriptAnimationSplitInfo::Internal_getname(ScriptAnimationSplitInfo* thisPtr)
	{
		String tmp__output;
		tmp__output = thisPtr->GetInternal()->name;

		MonoString* __output;
		__output = MonoUtil::stringToMono(tmp__output);

		return __output;
	}

	void ScriptAnimationSplitInfo::Internal_setname(ScriptAnimationSplitInfo* thisPtr, MonoString* value)
	{
		String tmpvalue;
		tmpvalue = MonoUtil::monoToString(value);
		thisPtr->GetInternal()->name = tmpvalue;
	}

	uint32_t ScriptAnimationSplitInfo::Internal_getstartFrame(ScriptAnimationSplitInfo* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->startFrame;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::Internal_setstartFrame(ScriptAnimationSplitInfo* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->startFrame = value;
	}

	uint32_t ScriptAnimationSplitInfo::Internal_getendFrame(ScriptAnimationSplitInfo* thisPtr)
	{
		uint32_t tmp__output;
		tmp__output = thisPtr->GetInternal()->endFrame;

		uint32_t __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::Internal_setendFrame(ScriptAnimationSplitInfo* thisPtr, uint32_t value)
	{
		thisPtr->GetInternal()->endFrame = value;
	}

	bool ScriptAnimationSplitInfo::Internal_getisAdditive(ScriptAnimationSplitInfo* thisPtr)
	{
		bool tmp__output;
		tmp__output = thisPtr->GetInternal()->isAdditive;

		bool __output;
		__output = tmp__output;

		return __output;
	}

	void ScriptAnimationSplitInfo::Internal_setisAdditive(ScriptAnimationSplitInfo* thisPtr, bool value)
	{
		thisPtr->GetInternal()->isAdditive = value;
	}
#endif
}
