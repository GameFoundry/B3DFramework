//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/RTTI/BsGUISkinRTTI.h"

namespace bs
{
	GUISkinEntry::GUISkinEntry(const String& name, const GUIElementStyle& style)
		:name(name), style(style)
	{ }

	RTTITypeBase* GUISkinEntry::getRTTIStatic()
	{
		return GUISkinEntryRTTI::Instance();
	}

	RTTITypeBase* GUISkinEntry::getRTTI() const
	{
		return GUISkinEntry::GetRTTIStatic();
	}
}
