//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "RTTI/BsColorRTTI.h"
#include "RTTI/BsRectOffsetRTTI.h"
#include "GUI/BsGUIElementStyle.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	class BS_EXPORT GUIElementStyleRTTI : public RTTIType<GUIElementStyle, IReflectable, GUIElementStyleRTTI>
	{
	private:
		BS_BEGIN_RTTI_MEMBERS
			BS_RTTI_MEMBER_REFL(Font, 0)
			BS_RTTI_MEMBER_PLAIN(FontSize, 1)
			BS_RTTI_MEMBER_PLAIN(TextHorzAlign, 2)
			BS_RTTI_MEMBER_PLAIN(TextVertAlign, 3)
			BS_RTTI_MEMBER_PLAIN(ImagePosition, 4)
			BS_RTTI_MEMBER_PLAIN(WordWrap, 5)

			BS_RTTI_MEMBER_REFL_NAMED(normalTex, Normal.Texture, 6)
			BS_RTTI_MEMBER_PLAIN_NAMED(normalTextColor, Normal.TextColor, 7)
			BS_RTTI_MEMBER_REFL_NAMED(hoverTex, Hover.Texture, 8)
			BS_RTTI_MEMBER_PLAIN_NAMED(hoverTextColor, Hover.TextColor, 9)
			BS_RTTI_MEMBER_REFL_NAMED(activeTex, Active.Texture, 10)
			BS_RTTI_MEMBER_PLAIN_NAMED(activeTextColor, Active.TextColor, 11)
			BS_RTTI_MEMBER_REFL_NAMED(focusedTex, Focused.Texture, 12)
			BS_RTTI_MEMBER_PLAIN_NAMED(focusedTextColor, Focused.TextColor, 13)

			BS_RTTI_MEMBER_REFL_NAMED(normalOnTex, NormalOn.Texture, 14)
			BS_RTTI_MEMBER_PLAIN_NAMED(normalOnTextColor, NormalOn.TextColor, 15)
			BS_RTTI_MEMBER_REFL_NAMED(hoverOnTex, HoverOn.Texture, 16)
			BS_RTTI_MEMBER_PLAIN_NAMED(hoverOnTextColor, HoverOn.TextColor, 17)
			BS_RTTI_MEMBER_REFL_NAMED(activeOnTex, ActiveOn.Texture, 18)
			BS_RTTI_MEMBER_PLAIN_NAMED(activeOnTextColor, ActiveOn.TextColor, 19)
			BS_RTTI_MEMBER_REFL_NAMED(focusedOnTex, FocusedOn.Texture, 20)
			BS_RTTI_MEMBER_PLAIN_NAMED(focusedOnTextColor, FocusedOn.TextColor, 21)

			BS_RTTI_MEMBER_PLAIN(Border, 22)
			BS_RTTI_MEMBER_PLAIN(Margins, 23)
			BS_RTTI_MEMBER_PLAIN(ContentOffset, 24)
			BS_RTTI_MEMBER_PLAIN(Padding, 25)

			BS_RTTI_MEMBER_PLAIN(Width, 26)
			BS_RTTI_MEMBER_PLAIN(Height, 27)
			BS_RTTI_MEMBER_PLAIN(MinWidth, 28)
			BS_RTTI_MEMBER_PLAIN(MaxWidth, 29)
			BS_RTTI_MEMBER_PLAIN(MinHeight, 30)
			BS_RTTI_MEMBER_PLAIN(MaxHeight, 31)
			BS_RTTI_MEMBER_PLAIN(FixedWidth, 32)
			BS_RTTI_MEMBER_PLAIN(FixedHeight, 33)

			BS_RTTI_MEMBER_PLAIN(SubStyles, 34)

			BS_RTTI_MEMBER_REFL_NAMED(focusedHoverTex, FocusedHover.Texture, 35)
			BS_RTTI_MEMBER_PLAIN_NAMED(focusedHoverTextColor, FocusedHover.TextColor, 36)
			BS_RTTI_MEMBER_REFL_NAMED(focusedHoverOnTex, FocusedHoverOn.Texture, 37)
			BS_RTTI_MEMBER_PLAIN_NAMED(focusedHoverOnTextColor, FocusedHoverOn.TextColor, 38)
		BS_END_RTTI_MEMBERS

	public:
		const String& GetRttiName() override
		{
			static String name = "GUIElementStyle";
			return name;
		}

		u32 GetRttiId() override
		{
			return TID_GUIElementStyle;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<GUIElementStyle>();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
