//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsSkeletonMask.h"
#include "BsSkeleton.h"

namespace BansheeEngine
{
	SkeletonMask::SkeletonMask(UINT32 numBones)
		:mIsDisabled(numBones)
	{ }

	bool SkeletonMask::isEnabled(UINT32 boneIdx) const
	{
		if (boneIdx >= (UINT32)mIsDisabled.size())
			return true;

		return !mIsDisabled[boneIdx];
	}

	SkeletonMaskBuilder::SkeletonMaskBuilder(const SPtr<Skeleton>& skeleton)
		:mSkeleton(skeleton), mMask(skeleton->getNumBones())
	{ }

	void SkeletonMaskBuilder::setBoneState(const String& name, bool enabled)
	{
		UINT32 numBones = mSkeleton->getNumBones();
		for(UINT32 i = 0; i < numBones; i++)
		{
			if(mSkeleton->getBoneInfo(i).name == name)
			{
				mMask.mIsDisabled[i] = !enabled;
				break;
			}
		}
	}
}