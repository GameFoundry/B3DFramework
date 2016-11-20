//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsPostProcessSettings.h"
#include "BsPostProcessSettingsRTTI.h"

namespace bs
{
	RTTITypeBase* PostProcessSettings::getRTTIStatic()
	{
		return PostProcessSettingsRTTI::instance();
	}

	RTTITypeBase* PostProcessSettings::getRTTI() const
	{
		return PostProcessSettings::getRTTIStatic();
	}
}