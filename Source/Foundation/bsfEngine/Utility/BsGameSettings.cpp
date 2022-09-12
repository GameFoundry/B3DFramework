//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsGameSettings.h"
#include "Private/RTTI/BsGameSettingsRTTI.h"

namespace bs
{
	RTTITypeBase* GameSettings::getRTTIStatic()
	{
		return GameSettingsRTTI::Instance();
	}

	RTTITypeBase* GameSettings::getRTTI() const
	{
		return GameSettings::GetRTTIStatic();
	}

}
