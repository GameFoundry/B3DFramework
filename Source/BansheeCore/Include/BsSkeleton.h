//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"
#include "BsMatrix4.h"

namespace BansheeEngine
{
	/** @addtogroup Animation-Internal
	 *  @{
	 */

	struct BONE_DESC
	{
		String name;
		UINT32 parent;

		Matrix4 invBindPose;
	};

	struct ANIMATION_STATE_DESC
	{
		const AnimationClip* clip;
		float weight;
		float speed;
		bool loop;
		UINT8 layer;
	};

	struct SkeletonPose
	{
		SkeletonPose(UINT32 numBones)
			:numBones(numBones), bonePoses(bs_newN<Matrix4>(numBones))
		{}

		~SkeletonPose()
		{
			bs_deleteN(bonePoses, numBones);
		}

		Matrix4* bonePoses;
		UINT32 numBones;
	};

	class Skeleton // Note: Must be immutable in order to be usable on multiple threads
	{
	public:
		Skeleton(BONE_DESC* bones, UINT32 numBones);
		~Skeleton();

		void getPose(SkeletonPose& pose, const AnimationClip& clip, float time, bool loop = true);
		void getPose(SkeletonPose& pose, const ANIMATION_STATE_DESC* states, UINT32 numStates, float time);

	private:
		struct BoneInfo
		{
			String name;
			UINT32 parent;
		};

		UINT32 mNumBones;
		Matrix4* mBindPoses;
		BoneInfo* mBoneInfo;
	};

	/** @} */
}