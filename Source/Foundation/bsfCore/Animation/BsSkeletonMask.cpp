//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsSkeletonMask.h"
#include "Animation/BsSkeleton.h"

using namespace bs;

SkeletonMask::SkeletonMask(u32 numBones)
	: mIsDisabled(numBones)
{}

bool SkeletonMask::IsEnabled(u32 boneIdx) const
{
	if(boneIdx >= (u32)mIsDisabled.size())
		return true;

	return !mIsDisabled[boneIdx];
}

SkeletonMaskBuilder::SkeletonMaskBuilder(const SPtr<Skeleton>& skeleton)
	: mSkeleton(skeleton), mMask(skeleton->GetNumBones())
{}

void SkeletonMaskBuilder::SetBoneState(const String& name, bool enabled)
{
	u32 numBones = mSkeleton->GetNumBones();
	for(u32 i = 0; i < numBones; i++)
	{
		if(mSkeleton->GetBoneInfo(i).Name == name)
		{
			mMask.mIsDisabled[i] = !enabled;
			break;
		}
	}
}
