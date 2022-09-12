//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Animation/BsSkeletonMask.h"
#include "Animation/BsSkeleton.h"

namespace bs
{
	SkeletonMask::SkeletonMask(UINT32 numBones)
		:mIsDisabled(numBones)
	{ }

	bool SkeletonMask::IsEnabled(UINT32 boneIdx) const
	{
		if (boneIdx >= (UINT32)mIsDisabled.size())
			return true;

		return !mIsDisabled[boneIdx];
	}

	SkeletonMaskBuilder::SkeletonMaskBuilder(const SPtr<Skeleton>& skeleton)
		:mSkeleton(skeleton), mMask(skeleton->GetNumBones())
	{ }

	void SkeletonMaskBuilder::SetBoneState(const String& name, bool enabled)
	{
		UINT32 numBones = mSkeleton->GetNumBones();
		for(UINT32 i = 0; i < numBones; i++)
		{
			if(mSkeleton->GetBoneInfo(i).name == name)
			{
				mMask.mIsDisabled[i] = !enabled;
				break;
			}
		}
	}
}
