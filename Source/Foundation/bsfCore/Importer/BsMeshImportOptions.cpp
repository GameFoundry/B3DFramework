//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsMeshImportOptions.h"
#include "Private/RTTI/BsMeshImportOptionsRTTI.h"

namespace bs
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

	SPtr<MeshImportOptions> MeshImportOptions::create()
	{
		return bs_shared_ptr_new<MeshImportOptions>();
	}

	RTTITypeBase* MeshImportOptions::getRTTIStatic()
	{
		return MeshImportOptionsRTTI::instance();
	}

	RTTITypeBase* MeshImportOptions::getRTTI() const
	{
		return MeshImportOptions::getRTTIStatic();
	}
}