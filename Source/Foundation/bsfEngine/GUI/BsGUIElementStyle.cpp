//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElementStyle.h"
#include "Private/RTTI/BsGUIElementStyleRTTI.h"

namespace bs
{
	RTTITypeBase* GUIElementStyle::GetRttiStatic()
	{
		return GUIElementStyleRTTI::Instance();
	}

	RTTITypeBase* GUIElementStyle::GetRtti() const
	{
		return GUIElementStyle::GetRttiStatic();
	}
} // namespace bs
