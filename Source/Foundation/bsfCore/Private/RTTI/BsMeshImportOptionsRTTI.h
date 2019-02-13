//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "Importer/BsMeshImportOptions.h"
#include "Private/RTTI/BsAnimationClipRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT MeshImportOptionsRTTI : public RTTIType <MeshImportOptions, ImportOptions, MeshImportOptionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(cpuCached, 0)
			BS_RTTI_MEMBER_PLAIN(importNormals, 1)
			BS_RTTI_MEMBER_PLAIN(importTangents, 2)
			BS_RTTI_MEMBER_PLAIN(importBlendShapes, 3)
			BS_RTTI_MEMBER_PLAIN(importSkin, 4)
			BS_RTTI_MEMBER_PLAIN(importAnimation, 5)
			BS_RTTI_MEMBER_PLAIN(importScale, 6)
			BS_RTTI_MEMBER_PLAIN(collisionMeshType, 7)
			BS_RTTI_MEMBER_REFL_ARRAY(animationSplits, 8)
			BS_RTTI_MEMBER_PLAIN(reduceKeyFrames, 9)
			BS_RTTI_MEMBER_REFL_ARRAY(animationEvents, 10)
			BS_RTTI_MEMBER_PLAIN(importRootMotion, 11)
		BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "MeshImportOptions";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_MeshImportOptions;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<MeshImportOptions>();
		}
	};

	class BS_CORE_EXPORT ImportedAnimationEventsRTTI : public RTTIType <ImportedAnimationEvents, IReflectable, ImportedAnimationEventsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(name, 0)
			BS_RTTI_MEMBER_PLAIN(events, 1)
		BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "ImportedAnimationEvents";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_ImportedAnimationEvents;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<ImportedAnimationEvents>();
		}
	};

	class BS_CORE_EXPORT AnimationSplitInfoRTTI : public RTTIType <AnimationSplitInfo, IReflectable, AnimationSplitInfoRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(name, 0)
			BS_RTTI_MEMBER_PLAIN(startFrame, 1)
			BS_RTTI_MEMBER_PLAIN(endFrame, 2)
			BS_RTTI_MEMBER_PLAIN(isAdditive, 3)
			BS_END_RTTI_MEMBERS
	public:
		const String& getRTTIName() override
		{
			static String name = "AnimationSplitInfo";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_AnimationSplitInfo;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<AnimationSplitInfo>();
		}
	};

	/** @} */
	/** @endcond */
}
