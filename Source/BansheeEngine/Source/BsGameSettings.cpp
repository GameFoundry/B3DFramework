//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGameSettings.h"
#include "BsGameSettingsRTTI.h"

namespace bs
{
	RTTITypeBase* GameSettings::getRTTIStatic()
	{
		return GameSettingsRTTI::instance();
	}

	RTTITypeBase* GameSettings::getRTTI() const
	{
		return GameSettings::getRTTIStatic();
	}

}