//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "BsGpuParamDesc.h"
#include "Private/RTTI/BsGpuProgramRTTI.h"

namespace bs
{
	RTTITypeBase* GpuParamDesc::getRTTIStatic()
	{
		return GpuParamDescRTTI::instance();
	}

	RTTITypeBase* GpuParamDesc::getRTTI() const
	{
		return GpuParamDesc::getRTTIStatic();
	}
}