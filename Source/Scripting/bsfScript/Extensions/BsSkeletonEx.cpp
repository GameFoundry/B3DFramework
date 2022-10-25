//********************************* bs::framework - Copyright 2018-2019 Marko Pintera ************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/BsSkeletonEx.h"

using namespace std::placeholders;

namespace bs
{
	SkeletonBoneInfoEx SkeletonEx::GetBoneInfo(const SPtr<Skeleton>& thisPtr, int boneIdx)
	{
		const SkeletonBoneInfo& boneInfo = thisPtr->GetBoneInfo(boneIdx);
		SkeletonBoneInfoEx boneInfoEx;
		boneInfoEx.Name = boneInfo.Name;
		boneInfoEx.Parent = boneInfo.Parent;
		boneInfoEx.InvBindPose = thisPtr->GetInvBindPose(boneIdx);

		return boneInfoEx;
	}
} // namespace bs
