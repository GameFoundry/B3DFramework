//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "Importer/BsShaderImportOptions.h"
#include "Private/RTTI/BsShaderImportOptionsRTTI.h"

namespace bs
{
	/************************************************************************/
	/* 								SERIALIZATION                      		*/
	/************************************************************************/
	RTTITypeBase* ShaderImportOptions::getRTTIStatic()
	{
		return ShaderImportOptionsRTTI::instance();
	}

	RTTITypeBase* ShaderImportOptions::getRTTI() const
	{
		return ShaderImportOptions::getRTTIStatic();
	}
}