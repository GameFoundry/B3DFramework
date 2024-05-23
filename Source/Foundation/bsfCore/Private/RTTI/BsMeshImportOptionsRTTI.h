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

	class B3D_CORE_EXPORT MeshImportOptionsRTTI : public RTTIType<MeshImportOptions, ImportOptions, MeshImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(CpuCached, 0)
			B3D_RTTI_MEMBER_PLAIN(ImportNormals, 1)
			B3D_RTTI_MEMBER_PLAIN(ImportTangents, 2)
			B3D_RTTI_MEMBER_PLAIN(ImportBlendShapes, 3)
			B3D_RTTI_MEMBER_PLAIN(ImportSkin, 4)
			B3D_RTTI_MEMBER_PLAIN(ImportAnimation, 5)
			B3D_RTTI_MEMBER_PLAIN(ImportScale, 6)
			B3D_RTTI_MEMBER_PLAIN(CollisionMeshType, 7)
			B3D_RTTI_MEMBER_REFL_ARRAY(AnimationSplits, 8)
			B3D_RTTI_MEMBER_PLAIN(ReduceKeyFrames, 9)
			B3D_RTTI_MEMBER_REFL_ARRAY(AnimationEvents, 10)
			B3D_RTTI_MEMBER_PLAIN(ImportRootMotion, 11)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "MeshImportOptions";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_MeshImportOptions;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<MeshImportOptions>();
		}
	};

	class B3D_CORE_EXPORT ImportedAnimationEventsRTTI : public RTTIType<ImportedAnimationEvents, IReflectable, ImportedAnimationEventsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Name, 0)
			B3D_RTTI_MEMBER(Events, 1)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "ImportedAnimationEvents";
			return name;
		}

		u32 GetRttiId() const override
		{
			return TID_ImportedAnimationEvents;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<ImportedAnimationEvents>();
		}
	};

	class B3D_CORE_EXPORT AnimationSplitInfoRTTI : public RTTIType<AnimationSplitInfo, IReflectable, AnimationSplitInfoRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Name, 0)
			B3D_RTTI_MEMBER(StartFrame, 1)
			B3D_RTTI_MEMBER(EndFrame, 2)
			B3D_RTTI_MEMBER(IsAdditive, 3)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "AnimationSplitInfo";
			return name;
		}

		u32 GetRttiId() const override
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
