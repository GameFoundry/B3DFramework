//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioClipImportOptions.h"
#include "Private/RTTI/BsAudioClipImportOptionsRTTI.h"

using namespace b3d;

SPtr<AudioClipImportOptions> AudioClipImportOptions::Create()
{
	return B3DMakeShared<AudioClipImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* AudioClipImportOptions::GetRttiStatic()
{
	return AudioClipImportOptionsRTTI::Instance();
}

RTTIType* AudioClipImportOptions::GetRtti() const
{
	return GetRttiStatic();
}
