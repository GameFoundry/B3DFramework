//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsSkeletonEx.h"

using namespace std::placeholders;

namespace bs
{
	SkeletonBoneInfoEx SkeletonEx::getBoneInfo(const SPtr<Skeleton>& thisPtr, int boneIdx)
	{
		const SkeletonBoneInfo& boneInfo = thisPtr->getBoneInfo(boneIdx);
		SkeletonBoneInfoEx boneInfoEx;
		boneInfoEx.name = boneInfo.name;
		boneInfoEx.parent = boneInfo.parent;
		boneInfoEx.invBindPose = thisPtr->getInvBindPose(boneIdx);

		return boneInfoEx;
	}
}