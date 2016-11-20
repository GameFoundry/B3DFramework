//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsGUISkinRTTI.h"

namespace bs
{
	GUISkinEntry::GUISkinEntry(const String& name, const GUIElementStyle& style)
		:name(name), style(style)
	{ }

	RTTITypeBase* GUISkinEntry::getRTTIStatic()
	{
		return GUISkinEntryRTTI::instance();
	}

	RTTITypeBase* GUISkinEntry::getRTTI() const
	{
		return GUISkinEntry::getRTTIStatic();
	}
}