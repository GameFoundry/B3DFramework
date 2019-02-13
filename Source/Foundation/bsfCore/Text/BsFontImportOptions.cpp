//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Text/BsFontImportOptions.h"
#include "Private/RTTI/BsFontImportOptionsRTTI.h"

namespace bs
{
	SPtr<FontImportOptions> FontImportOptions::create()
	{
		return bs_shared_ptr_new<FontImportOptions>();
	}

	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* FontImportOptions::getRTTIStatic()
	{
		return FontImportOptionsRTTI::instance();
	}

	RTTITypeBase* FontImportOptions::getRTTI() const
	{
		return FontImportOptions::getRTTIStatic();
	}
}