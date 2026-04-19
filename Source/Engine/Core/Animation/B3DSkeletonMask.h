//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	/** @addtogroup Animation
	 *  @{
	 */

	/**
	 * Contains a bitfield that determines which skeleton bones are enabled or disabled during skeletal animation. Use
	 * SkeletonMaskBuilder to create a mask for a specific skeleton.
	 */
	class B3D_EXPORT SkeletonMask
	{
	public:
		SkeletonMask() = default;
		SkeletonMask(u32 boneCount);

		/**
		 * Checks is the bone at the specified index enabled. Caller is expected to know which skeleton is the skeleton
		 * mask tied with, in order to determine the bone index.
		 */
		bool IsEnabled(u32 boneIndex) const;

	private:
		friend class SkeletonMaskBuilder;

		Vector<bool> mIsDisabled;
	};

	/** Builds a SkeletonMask for a specific skeleton. */
	class B3D_EXPORT SkeletonMaskBuilder
	{
	public:
		SkeletonMaskBuilder(const SPtr<Skeleton>& skeleton);

		/** Enables or disables a bone with the specified name. */
		void SetBoneState(const String& name, bool enabled);

		/** Teturns the built skeleton mask. */
		SkeletonMask GetMask() const { return mMask; }

	private:
		SPtr<Skeleton> mSkeleton;
		SkeletonMask mMask;
	};

	/** @} */
} // namespace b3d
