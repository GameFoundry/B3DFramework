//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsTextureImportOptions.h"
#include "Private/RTTI/BsTextureImportOptionsRTTI.h"

using namespace b3d;

SPtr<TextureImportOptions> TextureImportOptions::Create()
{
	return B3DMakeShared<TextureImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* TextureImportOptions::GetRttiStatic()
{
	return TextureImportOptionsRTTI::Instance();
}

RTTIType* TextureImportOptions::GetRtti() const
{
	return TextureImportOptions::GetRttiStatic();
}
