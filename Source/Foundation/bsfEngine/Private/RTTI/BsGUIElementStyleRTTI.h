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

	class B3D_EXPORT GUIElementStyleRTTI : public RTTIType<GUIElementStyle, IReflectable, GUIElementStyleRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL(Font, 0)
			B3D_RTTI_MEMBER_PLAIN(FontSize, 1)
			B3D_RTTI_MEMBER_PLAIN(TextHorzAlign, 2)
			B3D_RTTI_MEMBER_PLAIN(TextVertAlign, 3)
			B3D_RTTI_MEMBER_PLAIN(ImagePosition, 4)
			B3D_RTTI_MEMBER_PLAIN(WordWrap, 5)

			B3D_RTTI_MEMBER_REFL_NAMED(normalTex, Normal.Texture, 6)
			B3D_RTTI_MEMBER_PLAIN_NAMED(normalTextColor, Normal.TextColor, 7)
			B3D_RTTI_MEMBER_REFL_NAMED(hoverTex, Hover.Texture, 8)
			B3D_RTTI_MEMBER_PLAIN_NAMED(hoverTextColor, Hover.TextColor, 9)
			B3D_RTTI_MEMBER_REFL_NAMED(activeTex, Active.Texture, 10)
			B3D_RTTI_MEMBER_PLAIN_NAMED(activeTextColor, Active.TextColor, 11)
			B3D_RTTI_MEMBER_REFL_NAMED(focusedTex, Focused.Texture, 12)
			B3D_RTTI_MEMBER_PLAIN_NAMED(focusedTextColor, Focused.TextColor, 13)

			B3D_RTTI_MEMBER_REFL_NAMED(normalOnTex, NormalOn.Texture, 14)
			B3D_RTTI_MEMBER_PLAIN_NAMED(normalOnTextColor, NormalOn.TextColor, 15)
			B3D_RTTI_MEMBER_REFL_NAMED(hoverOnTex, HoverOn.Texture, 16)
			B3D_RTTI_MEMBER_PLAIN_NAMED(hoverOnTextColor, HoverOn.TextColor, 17)
			B3D_RTTI_MEMBER_REFL_NAMED(activeOnTex, ActiveOn.Texture, 18)
			B3D_RTTI_MEMBER_PLAIN_NAMED(activeOnTextColor, ActiveOn.TextColor, 19)
			B3D_RTTI_MEMBER_REFL_NAMED(focusedOnTex, FocusedOn.Texture, 20)
			B3D_RTTI_MEMBER_PLAIN_NAMED(focusedOnTextColor, FocusedOn.TextColor, 21)

			B3D_RTTI_MEMBER_PLAIN(Border, 22)
			B3D_RTTI_MEMBER_PLAIN(Margins, 23)
			B3D_RTTI_MEMBER_PLAIN(ContentOffset, 24)
			B3D_RTTI_MEMBER_PLAIN(Padding, 25)

			B3D_RTTI_MEMBER_PLAIN(Width, 26)
			B3D_RTTI_MEMBER_PLAIN(Height, 27)
			B3D_RTTI_MEMBER_PLAIN(MinWidth, 28)
			B3D_RTTI_MEMBER_PLAIN(MaxWidth, 29)
			B3D_RTTI_MEMBER_PLAIN(MinHeight, 30)
			B3D_RTTI_MEMBER_PLAIN(MaxHeight, 31)
			B3D_RTTI_MEMBER_PLAIN(FixedWidth, 32)
			B3D_RTTI_MEMBER_PLAIN(FixedHeight, 33)

			B3D_RTTI_MEMBER_PLAIN(SubStyles, 34)

			B3D_RTTI_MEMBER_REFL_NAMED(focusedHoverTex, FocusedHover.Texture, 35)
			B3D_RTTI_MEMBER_PLAIN_NAMED(focusedHoverTextColor, FocusedHover.TextColor, 36)
			B3D_RTTI_MEMBER_REFL_NAMED(focusedHoverOnTex, FocusedHoverOn.Texture, 37)
			B3D_RTTI_MEMBER_PLAIN_NAMED(focusedHoverOnTextColor, FocusedHoverOn.TextColor, 38)
		B3D_RTTI_END_MEMBERS

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
