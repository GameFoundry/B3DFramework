//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioClipImportOptions.h"
#include "Private/RTTI/BsAudioClipImportOptionsRTTI.h"

using namespace bs;

SPtr<AudioClipImportOptions> AudioClipImportOptions::Create()
{
	return B3DMakeShared<AudioClipImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* AudioClipImportOptions::GetRttiStatic()
{
	return AudioClipImportOptionsRTTI::Instance();
}

RTTITypeBase* AudioClipImportOptions::GetRtti() const
{
	return GetRttiStatic();
}
