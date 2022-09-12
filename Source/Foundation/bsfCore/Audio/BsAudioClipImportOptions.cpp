//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Audio/BsAudioClipImportOptions.h"
#include "Private/RTTI/BsAudioClipImportOptionsRTTI.h"

namespace bs
{
	SPtr<AudioClipImportOptions> AudioClipImportOptions::Create()
	{
		return bs_shared_ptr_new<AudioClipImportOptions>();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* AudioClipImportOptions::getRTTIStatic()
	{
		return AudioClipImportOptionsRTTI::Instance();
	}

	RTTITypeBase* AudioClipImportOptions::getRTTI() const
	{
		return GetRTTIStatic();
	}
}
