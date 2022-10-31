//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsCorePrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "Importer/BsMeshImportOptions.h"
#include "Private/RTTI/BsAnimationClipRTTI.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Core
	 *  @{
	 */

	class BS_CORE_EXPORT MeshImportOptionsRTTI : public RTTIType<MeshImportOptions, ImportOptions, MeshImportOptionsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(CpuCached, 0)
			BS_RTTI_MEMBER_PLAIN(ImportNormals, 1)
			BS_RTTI_MEMBER_PLAIN(ImportTangents, 2)
			BS_RTTI_MEMBER_PLAIN(ImportBlendShapes, 3)
			BS_RTTI_MEMBER_PLAIN(ImportSkin, 4)
			BS_RTTI_MEMBER_PLAIN(ImportAnimation, 5)
			BS_RTTI_MEMBER_PLAIN(ImportScale, 6)
			BS_RTTI_MEMBER_PLAIN(CollisionMeshType, 7)
			BS_RTTI_MEMBER_REFL_ARRAY(AnimationSplits, 8)
			BS_RTTI_MEMBER_PLAIN(ReduceKeyFrames, 9)
			BS_RTTI_MEMBER_REFL_ARRAY(AnimationEvents, 10)
			BS_RTTI_MEMBER_PLAIN(ImportRootMotion, 11)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "MeshImportOptions";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_MeshImportOptions;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<MeshImportOptions>();
		}
	};

	class BS_CORE_EXPORT ImportedAnimationEventsRTTI : public RTTIType<ImportedAnimationEvents, IReflectable, ImportedAnimationEventsRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Name, 0)
			BS_RTTI_MEMBER_PLAIN(Events, 1)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "ImportedAnimationEvents";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_ImportedAnimationEvents;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ImportedAnimationEvents>();
		}
	};

	class BS_CORE_EXPORT AnimationSplitInfoRTTI : public RTTIType<AnimationSplitInfo, IReflectable, AnimationSplitInfoRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_PLAIN(Name, 0)
			BS_RTTI_MEMBER_PLAIN(StartFrame, 1)
			BS_RTTI_MEMBER_PLAIN(EndFrame, 2)
			BS_RTTI_MEMBER_PLAIN(IsAdditive, 3)
		BS_END_RTTI_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "AnimationSplitInfo";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_AnimationSplitInfo;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<AnimationSplitInfo>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
