//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFontImportOptions.h"
#include "Private/RTTI/BsFontImportOptionsRTTI.h"

using namespace bs;

SPtr<FontImportOptions> FontImportOptions::Create()
{
	return bs_shared_ptr_new<FontImportOptions>();
}

/************************************************************************/
/* 								SERIALIZATION                      		*/
/************************************************************************/
RTTITypeBase* FontImportOptions::GetRttiStatic()
{
	return FontImportOptionsRTTI::Instance();
}

RTTITypeBase* FontImportOptions::GetRtti() const
{
	return FontImportOptions::GetRttiStatic();
}
