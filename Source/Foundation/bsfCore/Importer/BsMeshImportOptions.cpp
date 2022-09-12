//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsMeshImportOptions.h"
#include "Private/RTTI/BsMeshImportOptionsRTTI.h"

namespace bs
{
	RTTITypeBase* AnimationSplitInfo::getRTTIStatic()
	{
		return AnimationSplitInfoRTTI::Instance();
	}

	RTTITypeBase* AnimationSplitInfo::getRTTI() const
	{
		return AnimationSplitInfo::GetRTTIStatic();
	}

	RTTITypeBase* ImportedAnimationEvents::getRTTIStatic()
	{
		return ImportedAnimationEventsRTTI::Instance();
	}

	RTTITypeBase* ImportedAnimationEvents::getRTTI() const
	{
		return ImportedAnimationEvents::GetRTTIStatic();
	}

	SPtr<MeshImportOptions> MeshImportOptions::Create()
	{
		return bs_shared_ptr_new<MeshImportOptions>();
	}

	RTTITypeBase* MeshImportOptions::getRTTIStatic()
	{
		return MeshImportOptionsRTTI::Instance();
	}

	RTTITypeBase* MeshImportOptions::getRTTI() const
	{
		return MeshImportOptions::GetRTTIStatic();
	}
}
