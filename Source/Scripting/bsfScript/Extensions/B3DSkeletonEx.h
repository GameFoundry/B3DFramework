//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DScriptEnginePrerequisites.h"
#include "Animation/B3DSkeleton.h"

namespace b3d
{
	/** @addtogroup ScriptInteropEngine
	 *  @{
	 */
	/** @cond SCRIPT_EXTENSIONS */

	/** Contains internal information about a single bone in a Skeleton. */
	struct B3D_SCRIPT_EXPORT(ExportName(BoneInfo), ExportAsStruct(true)) SkeletonBoneInfoEx
	{
		/** Unique name of the bone. */
		String Name;

		/** Index of the parent bone (within the relevant Skeleton object). -1 if root bone. */
		int Parent;

		/** Inverse transform of the pose the skeleton was initially created in. */
		Matrix4 InvBindPose;
	};

	/** Extension class for Skeleton, for adding additional functionality for the script version of the class. */
	class B3D_SCRIPT_EXPORT(ExtensionClassForType(Skeleton)) SkeletonEx
	{
	public:
		/** Returns information about a bone at the provided index.
		 *
		 * @param[in] boneIdx	Index of the bone to retrieve information for.
		 * @return				Information about the bone at the specified index.
		 */
		B3D_SCRIPT_EXPORT(ExtensionMethodForType(Skeleton), ExportName(GetBoneInfo))
		static SkeletonBoneInfoEx GetBoneInfo(const TShared<Skeleton>& thisPtr, int boneIdx);
	};

	/** @endcond */
	/** @} */
} // namespace b3d
