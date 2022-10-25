//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsMeshImportOptions.h"
#include "Private/RTTI/BsMeshImportOptionsRTTI.h"

using namespace bs;

RTTITypeBase* AnimationSplitInfo::GetRttiStatic()
{
	return AnimationSplitInfoRTTI::Instance();
}

RTTITypeBase* AnimationSplitInfo::GetRtti() const
{
	return AnimationSplitInfo::GetRttiStatic();
}

RTTITypeBase* ImportedAnimationEvents::GetRttiStatic()
{
	return ImportedAnimationEventsRTTI::Instance();
}

RTTITypeBase* ImportedAnimationEvents::GetRtti() const
{
	return ImportedAnimationEvents::GetRttiStatic();
}

SPtr<MeshImportOptions> MeshImportOptions::Create()
{
	return bs_shared_ptr_new<MeshImportOptions>();
}

RTTITypeBase* MeshImportOptions::GetRttiStatic()
{
	return MeshImportOptionsRTTI::Instance();
}

RTTITypeBase* MeshImportOptions::GetRtti() const
{
	return MeshImportOptions::GetRttiStatic();
}
