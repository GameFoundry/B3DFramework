//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/B3DFontImportOptions.h"
#include "RTTI/B3DFontImportOptionsRTTI.h"

using namespace b3d;

SPtr<FontImportOptions> FontImportOptions::Create()
{
	return B3DMakeShared<FontImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTIType* FontImportOptions::GetRttiStatic()
{
	return FontImportOptionsRTTI::Instance();
}

RTTIType* FontImportOptions::GetRtti() const
{
	return FontImportOptions::GetRttiStatic();
}
