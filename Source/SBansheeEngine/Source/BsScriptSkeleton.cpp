//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsScriptSkeleton.h"
#include "BsScriptMeta.h"
#include "BsMonoField.h"
#include "BsMonoMethod.h"
#include "BsMonoClass.h"
#include "BsMonoManager.h"
#include "BsMonoUtil.h"

using namespace std::placeholders;

namespace BansheeEngine
{
	ScriptSkeleton::ScriptSkeleton(MonoObject* managedInstance, const SPtr<Skeleton>& skeleton)
		:ScriptObject(managedInstance), mSkeleton(skeleton)
	{

	}

	void ScriptSkeleton::initRuntimeData()
	{
		metaData.scriptClass->addInternalCall("Internal_GetNumBones", &ScriptSkeleton::internal_GetNumBones);
		metaData.scriptClass->addInternalCall("Internal_GetBoneInfo", &ScriptSkeleton::internal_GetBoneInfo);
	}

	MonoObject* ScriptSkeleton::create(const SPtr<Skeleton>& skeleton)
	{
		MonoObject* instance = metaData.scriptClass->createInstance();

		new (bs_alloc<ScriptSkeleton>()) ScriptSkeleton(instance, skeleton);
		return instance;
	}

	int ScriptSkeleton::internal_GetNumBones(ScriptSkeleton* thisPtr)
	{
		return thisPtr->getInternal()->getNumBones();
	}

	MonoObject* ScriptSkeleton::internal_GetBoneInfo(ScriptSkeleton* thisPtr, UINT32 boneIdx)
	{
		SPtr<Skeleton> skeleton = thisPtr->getInternal();

		UINT32 numBones = skeleton->getNumBones();
		if (boneIdx >= numBones)
			return nullptr;

		const SkeletonBoneInfo& boneInfo = skeleton->getBoneInfo(boneIdx);
		const Matrix4& invBindPose = skeleton->getInvBindPose(boneIdx);

		return ScriptBoneInfo::toManaged(boneInfo, invBindPose);
	}
	
	MonoField* ScriptBoneInfo::sNameField = nullptr;
	MonoField* ScriptBoneInfo::sParentField = nullptr;
	MonoField* ScriptBoneInfo::sInvBindPoseField = nullptr;

	ScriptBoneInfo::ScriptBoneInfo(MonoObject* instance)
		:ScriptObject(instance)
	{ }

	void ScriptBoneInfo::initRuntimeData()
	{
		sNameField = metaData.scriptClass->getField("Name");
		sParentField = metaData.scriptClass->getField("Parent");
		sInvBindPoseField = metaData.scriptClass->getField("InvBindPose");
	}

	MonoObject* ScriptBoneInfo::toManaged(const SkeletonBoneInfo& boneInfo, const Matrix4& invBindPose)
	{
		MonoString* monoName = MonoUtil::stringToMono(boneInfo.name);
		int parentIdx = boneInfo.parent;
		Matrix4 monoInvBindPose = invBindPose;

		MonoObject* instance = metaData.scriptClass->createInstance();
		sNameField->setValue(instance, monoName);
		sParentField->setValue(instance, &parentIdx);
		sInvBindPoseField->setValue(instance, &monoInvBindPose);

		return instance;
	}
}