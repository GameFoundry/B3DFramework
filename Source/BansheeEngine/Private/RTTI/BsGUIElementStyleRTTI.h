//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "GUI/BsGUIElementStyle.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT GUIElementStyleRTTI : public RTTIType <GUIElementStyle, IReflectable, GUIElementStyleRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(font, 0)
			BS_RTTI_MEMBER_PLAIN(fontSize, 1)
			BS_RTTI_MEMBER_PLAIN(textHorzAlign, 2)
			BS_RTTI_MEMBER_PLAIN(textVertAlign, 3)
			BS_RTTI_MEMBER_PLAIN(imagePosition, 4)
			BS_RTTI_MEMBER_PLAIN(wordWrap, 5)

			BS_RTTI_MEMBER_REFL_NAMED(normalTex, normal.texture, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(normalTextColor, normal.textColor, 7)
			BS_RTTI_MEMBER_REFL_NAMED(hoverTex, hover.texture, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(hoverTextColor, hover.textColor, 9)
			BS_RTTI_MEMBER_REFL_NAMED(activeTex, active.texture, 10)
			BS_RTTI_MEMBER_PLAIN_NAMED(activeTextColor, active.textColor, 11)
			BS_RTTI_MEMBER_REFL_NAMED(focusedTex, focused.texture, 12)
			BS_RTTI_MEMBER_PLAIN_NAMED(focusedTextColor, focused.textColor, 13)

			BS_RTTI_MEMBER_REFL_NAMED(normalOnTex, normalOn.texture, 14)
			BS_RTTI_MEMBER_PLAIN_NAMED(normalOnTextColor, normalOn.textColor, 15)
			BS_RTTI_MEMBER_REFL_NAMED(hoverOnTex, hoverOn.texture, 16)
			BS_RTTI_MEMBER_PLAIN_NAMED(hoverOnTextColor, hoverOn.textColor, 17)
			BS_RTTI_MEMBER_REFL_NAMED(activeOnTex, activeOn.texture, 18)
			BS_RTTI_MEMBER_PLAIN_NAMED(activeOnTextColor, activeOn.textColor, 19)
			BS_RTTI_MEMBER_REFL_NAMED(focusedOnTex, focusedOn.texture, 20)
			BS_RTTI_MEMBER_PLAIN_NAMED(focusedOnTextColor, focusedOn.textColor, 21)

			BS_RTTI_MEMBER_PLAIN(border, 22)
			BS_RTTI_MEMBER_PLAIN(margins, 23)
			BS_RTTI_MEMBER_PLAIN(contentOffset, 24)
			BS_RTTI_MEMBER_PLAIN(padding, 25)

			BS_RTTI_MEMBER_PLAIN(width, 26)
			BS_RTTI_MEMBER_PLAIN(height, 27)
			BS_RTTI_MEMBER_PLAIN(minWidth, 28)
			BS_RTTI_MEMBER_PLAIN(maxWidth, 29)
			BS_RTTI_MEMBER_PLAIN(minHeight, 30)
			BS_RTTI_MEMBER_PLAIN(maxHeight, 31)
			BS_RTTI_MEMBER_PLAIN(fixedWidth, 32)
			BS_RTTI_MEMBER_PLAIN(fixedHeight, 33)

			BS_RTTI_MEMBER_PLAIN(subStyles, 34)
		BS_END_RTTI_MEMBERS
			
	public:
		GUIElementStyleRTTI()
			:mInitMembers(this)
		{ }

		const String& getRTTIName() override
		{
			static String name = "GUIElementStyle";
			return name;
		}

		UINT32 getRTTIId() override
		{
			return TID_GUIElementStyle;
		}

		SPtr<IReflectable> newRTTIObject() override
		{
			return bs_shared_ptr_new<GUIElementStyle>();
		}
	};

	/** @} */
	/** @endcond */
}