//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "Private/RTTI/BsGUISkinRTTI.h"

namespace bs
{
	GUISkinEntry::GUISkinEntry(const String& name, const GUIElementStyle& style)
		: Name(name), Style(style)
	{}

	RTTITypeBase* GUISkinEntry::GetRttiStatic()
	{
		return GUISkinEntryRTTI::Instance();
	}

	RTTITypeBase* GUISkinEntry::GetRtti() const
	{
		return GUISkinEntry::GetRttiStatic();
	}
} // namespace bs
