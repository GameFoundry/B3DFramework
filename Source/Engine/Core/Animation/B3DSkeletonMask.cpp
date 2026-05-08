//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/B3DSkeletonMask.h"
#include "Animation/B3DSkeleton.h"

using namespace b3d;

SkeletonMask::SkeletonMask(u32 boneCount)
	: mIsDisabled(boneCount)
{}

bool SkeletonMask::IsEnabled(u32 boneIndex) const
{
	if(boneIndex >= (u32)mIsDisabled.size())
		return true;

	return !mIsDisabled[boneIndex];
}

SkeletonMaskBuilder::SkeletonMaskBuilder(const TShared<Skeleton>& skeleton)
	: mSkeleton(skeleton), mMask(skeleton->GetBoneCount())
{}

void SkeletonMaskBuilder::SetBoneState(const String& name, bool enabled)
{
	u32 boneCount = mSkeleton->GetBoneCount();
	for(u32 i = 0; i < boneCount; i++)
	{
		if(mSkeleton->GetBoneInfo(i).Name == name)
		{
			mMask.mIsDisabled[i] = !enabled;
			break;
		}
	}
}
