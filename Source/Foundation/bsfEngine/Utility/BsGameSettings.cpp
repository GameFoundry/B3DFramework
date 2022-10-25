//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/BsGameSettings.h"
#include "Private/RTTI/BsGameSettingsRTTI.h"

using namespace bs;

RTTITypeBase* GameSettings::GetRttiStatic()
{
	return GameSettingsRTTI::Instance();
}

RTTITypeBase* GameSettings::GetRtti() const
{
	return GameSettings::GetRttiStatic();
}

