//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsTextureImportOptions.h"
#include "Private/RTTI/BsTextureImportOptionsRTTI.h"

using namespace bs;

SPtr<TextureImportOptions> TextureImportOptions::Create()
{
	return bs_shared_ptr_new<TextureImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* TextureImportOptions::GetRttiStatic()
{
	return TextureImportOptionsRTTI::Instance();
}

RTTITypeBase* TextureImportOptions::GetRtti() const
{
	return TextureImportOptions::GetRttiStatic();
}
