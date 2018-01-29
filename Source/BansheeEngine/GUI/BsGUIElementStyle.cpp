//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "GUI/BsGUIElementStyle.h"
#include "Private/RTTI/BsGUIElementStyleRTTI.h"

namespace bs
{
	RTTITypeBase* GUIElementStyle::getRTTIStatic()
	{
		return GUIElementStyleRTTI::instance();
	}

	RTTITypeBase* GUIElementStyle::getRTTI() const
	{
		return GUIElementStyle::getRTTIStatic();
	}
}