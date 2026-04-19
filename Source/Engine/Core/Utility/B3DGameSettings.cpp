//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Utility/B3DGameSettings.h"
#include "RTTI/B3DGameSettingsRTTI.h"

using namespace b3d;

RTTIType* GameSettings::GetRttiStatic()
{
	return GameSettingsRTTI::Instance();
}

RTTIType* GameSettings::GetRtti() const
{
	return GameSettings::GetRttiStatic();
}

