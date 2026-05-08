//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/B3DMeshImportOptions.h"
#include "RTTI/B3DMeshImportOptionsRTTI.h"

using namespace b3d;

RTTIType* AnimationSplitInfo::GetRttiStatic()
{
	return AnimationSplitInfoRTTI::Instance();
}

RTTIType* AnimationSplitInfo::GetRtti() const
{
	return AnimationSplitInfo::GetRttiStatic();
}

RTTIType* ImportedAnimationEvents::GetRttiStatic()
{
	return ImportedAnimationEventsRTTI::Instance();
}

RTTIType* ImportedAnimationEvents::GetRtti() const
{
	return ImportedAnimationEvents::GetRttiStatic();
}

TShared<MeshImportOptions> MeshImportOptions::Create()
{
	return B3DMakeShared<MeshImportOptions>();
}

RTTIType* MeshImportOptions::GetRttiStatic()
{
	return MeshImportOptionsRTTI::Instance();
}

RTTIType* MeshImportOptions::GetRtti() const
{
	return MeshImportOptions::GetRttiStatic();
}
