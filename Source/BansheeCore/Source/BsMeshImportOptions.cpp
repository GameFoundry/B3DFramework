//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsMeshImportOptions.h"
#include "BsMeshImportOptionsRTTI.h"

namespace BansheeEngine
{
	RTTITypeBase* AnimationSplitInfo::getRTTIStatic()
	{
		return AnimationSplitInfoRTTI::instance();
	}

	RTTITypeBase* AnimationSplitInfo::getRTTI() const
	{
		return AnimationSplitInfo::getRTTIStatic();
	}

	RTTITypeBase* ImportedAnimationEvents::getRTTIStatic()
	{
		return ImportedAnimationEventsRTTI::instance();
	}

	RTTITypeBase* ImportedAnimationEvents::getRTTI() const
	{
		return ImportedAnimationEvents::getRTTIStatic();
	}

	MeshImportOptions::MeshImportOptions()
		: mCPUReadable(false), mImportNormals(true), mImportTangents(true), mImportBlendShapes(false), mImportSkin(false)
		, mImportAnimation(false), mReduceKeyFrames(true), mImportRootMotion(false), mImportScale(1.0f)
		, mCollisionMeshType(CollisionMeshType::None)
	{ }

	RTTITypeBase* MeshImportOptions::getRTTIStatic()
	{
		return MeshImportOptionsRTTI::instance();
	}

	RTTITypeBase* MeshImportOptions::getRTTI() const
	{
		return MeshImportOptions::getRTTIStatic();
	}
}