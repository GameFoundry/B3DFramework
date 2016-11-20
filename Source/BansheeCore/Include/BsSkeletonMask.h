//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace bs
{
	/** @addtogroup Animation
	 *  @{
	 */

	/** 
	 * Contains a bitfield that determines which skeleton bones are enabled or disabled during skeletal animation. Use
	 * SkeletonMaskBuilder to create a mask for a specific skeleton.
	 */
	class BS_CORE_EXPORT SkeletonMask
	{
	public:
		SkeletonMask() {}
		SkeletonMask(UINT32 numBones);

		/** 
		 * Checks is the bone at the specified index enabled. Caller is expected to know which skeleton is the skeleton
		 * mask tied with, in order to determine the bone index. 
		 */
		bool isEnabled(UINT32 boneIdx) const;

	private:
		friend class SkeletonMaskBuilder;

		Vector<bool> mIsDisabled;
	};

	/** Builds a SkeletonMask for a specific skeleton. */
	class BS_CORE_EXPORT SkeletonMaskBuilder
	{
	public:
		SkeletonMaskBuilder(const SPtr<Skeleton>& skeleton);

		/** Enables or disables a bone with the specified name. */
		void setBoneState(const String& name, bool enabled);

		/** Teturns the built skeleton mask. */
		SkeletonMask getMask() const { return mMask; }

	private:
		SPtr<Skeleton> mSkeleton;
		SkeletonMask mMask;
	};

	/** @} */
}