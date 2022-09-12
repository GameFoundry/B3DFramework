//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Importer/BsShaderImportOptions.h"
#include "Private/RTTI/BsShaderImportOptionsRTTI.h"

namespace bs
{
	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* ShaderImportOptions::getRTTIStatic()
	{
		return ShaderImportOptionsRTTI::Instance();
	}

	RTTITypeBase* ShaderImportOptions::getRTTI() const
	{
		return ShaderImportOptions::GetRTTIStatic();
	}
}
