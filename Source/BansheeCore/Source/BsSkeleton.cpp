//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSkeleton.h"

namespace BansheeEngine
{
	Skeleton::Skeleton(BONE_DESC* bones, UINT32 numBones)
		:mBindPoses(bs_newN<Matrix4>(numBones)), mBoneInfo(bs_newN<BoneInfo>(numBones)), mNumBones(numBones)
	{
		for(UINT32 i = 0; i < numBones; i++)
		{
			mBindPoses[i] = bones[i].invBindPose;
			mBoneInfo[i].name = bones[i].name;
			mBoneInfo[i].parent = bones[i].parent;
		}
	}

	Skeleton::~Skeleton()
	{
		bs_deleteN(mBindPoses, mNumBones);
		bs_deleteN(mBoneInfo, mNumBones);
	}

	void Skeleton::getPose(SkeletonPose& pose, const AnimationClip& clip, float time, bool loop)
	{
		ANIMATION_STATE_DESC state;
		state.clip = &clip;
		state.layer = 0;
		state.loop = loop;
		state.speed = 1.0f;
		state.weight = 1.0f;

		getPose(pose, &state, 1, time);
	}

	void Skeleton::getPose(SkeletonPose& pose, const ANIMATION_STATE_DESC* states, UINT32 numStates, float time)
	{
		// TODO -Blend locally, normalize all weights to 1, layers for additive animations

		// TODO
	}
}