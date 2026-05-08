//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Extensions/B3DSkeletonEx.h"

using namespace b3d;
SkeletonBoneInfoEx SkeletonEx::GetBoneInfo(const TShared<Skeleton>& thisPtr, int boneIdx)
{
	const SkeletonBoneInfo& boneInfo = thisPtr->GetBoneInfo(boneIdx);
	SkeletonBoneInfoEx boneInfoEx;
	boneInfoEx.Name = boneInfo.Name;
	boneInfoEx.Parent = boneInfo.Parent;
	boneInfoEx.InvBindPose = thisPtr->GetInvBindPose(boneIdx);

	return boneInfoEx;
}
