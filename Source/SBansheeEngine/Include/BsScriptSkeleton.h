//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsScriptEnginePrerequisites.h"
#include "BsScriptObject.h"
#include "BsSkeleton.h"

namespace bs
{
	class ScriptAnimationClip;

	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */

	/**	Interop class between C++ & CLR for Skeleton. */
	class BS_SCR_BE_EXPORT ScriptSkeleton : public ScriptObject <ScriptSkeleton>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "Skeleton")

		/**	Returns the native wrapped animation. */
		SPtr<Skeleton> getInternal() const { return mSkeleton; }

		/**	Creates a new managed Skeleton instance wrapping the provided native skeleton. */
		static MonoObject* create(const SPtr<Skeleton>& skeleton);

	private:
		ScriptSkeleton(MonoObject* managedInstance, const SPtr<Skeleton>& skeleton);

		SPtr<Skeleton> mSkeleton;

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static int internal_GetNumBones(ScriptSkeleton* thisPtr);
		static MonoObject* internal_GetBoneInfo(ScriptSkeleton* thisPtr, UINT32 boneIdx);
	};

	/** Helper class for dealing with SkeletonBoneInfo structure. */
	class BS_SCR_BE_EXPORT ScriptBoneInfo : public ScriptObject<ScriptBoneInfo>
	{
	public:
		SCRIPT_OBJ(ENGINE_ASSEMBLY, "BansheeEngine", "BoneInfo")

		/** Converts native bone info to its native counterpart. */
		static MonoObject* toManaged(const SkeletonBoneInfo& boneInfo, const Matrix4& invBindPose);

	private:
		ScriptBoneInfo(MonoObject* instance);

		/************************************************************************/
		/* 								CLR HOOKS						   		*/
		/************************************************************************/
		static MonoField* sNameField;
		static MonoField* sParentField;
		static MonoField* sInvBindPoseField;
	};
	
	/** @} */
}