//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "Importer/B3DMeshImportOptions.h"
#include "RTTI/B3DAnimationClipRTTI.h"

namespace b3d
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class B3D_EXPORT MeshImportOptionsRTTI : public TRTTIType<MeshImportOptions, ImportOptions, MeshImportOptionsRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(CpuCached, 0)
			B3D_RTTI_MEMBER(ImportNormals, 1)
			B3D_RTTI_MEMBER(ImportTangents, 2)
			B3D_RTTI_MEMBER(ImportBlendShapes, 3)
			B3D_RTTI_MEMBER(ImportSkin, 4)
			B3D_RTTI_MEMBER(ImportAnimation, 5)
			B3D_RTTI_MEMBER(ImportScale, 6)
			B3D_RTTI_MEMBER(CollisionMeshType, 7)
			B3D_RTTI_MEMBER_CONTAINER(AnimationSplits, 8)
			B3D_RTTI_MEMBER(ReduceKeyFrames, 9)
			B3D_RTTI_MEMBER_CONTAINER(AnimationEvents, 10)
			B3D_RTTI_MEMBER(ImportRootMotion, 11)
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

	class B3D_EXPORT ImportedAnimationEventsRTTI : public TRTTIType<ImportedAnimationEvents, IReflectable, ImportedAnimationEventsRTTI>
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

	class B3D_EXPORT AnimationSplitInfoRTTI : public TRTTIType<AnimationSplitInfo, IReflectable, AnimationSplitInfoRTTI>
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
} // namespace b3d
