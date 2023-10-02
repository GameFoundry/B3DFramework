//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Reflection/BsRTTIType.h"
#include "RTTI/BsStringRTTI.h"
#include "RTTI/BsBitfieldRTTI.h"
#include "RTTI/BsRectOffsetRTTI.h"
#include "RTTI/BsMathRTTI.h"
#include "RTTI/BsColorRTTI.h"
#include "RTTI/BsStdRTTI.h"
#include "GUI/StyleSheet/BsGUIStyleSheet.h"

namespace bs
{
	/** @cond RTTI */
	/** @addtogroup RTTI-Impl-Engine
	 *  @{
	 */

	template<>
	struct RTTIPlainType<GUIStyleSheetBorderElement> : RTTIPlainTypeHelper<GUIStyleSheetBorderElement, TID_GUIStyleSheetBorderElement, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetBorderElement& object, Processor& processor, u8 version)
		{
			processor(object.Color);
			processor(object.Style);
			processor(object.Width);
		}
	};

	template<>
	struct RTTIPlainType<GUIStyleSheetStateStyle> : RTTIPlainTypeHelper<GUIStyleSheetStateStyle, TID_GUIStyleSheetStyleState, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetStateStyle& object, Processor& processor, u8 version)
		{
			processor(object.Selector);
			processor(object.PseudoClass);
			processor(object.SelectorType);

			processor(object.Margins);
			processor(object.Padding);

			processor(object.Size);
			processor(object.MinimumSize);
			processor(object.MaximumSize);

			processor(object.BackgroundColor);
			processor(object.Color);
			processor(object.Opacity);

			processor(object.BorderLeft);
			processor(object.BorderRight);
			processor(object.BorderTop);
			processor(object.BorderBottom);

			processor(object.BorderTopLeftRadius);
			processor(object.BorderTopRightRadius);
			processor(object.BorderBottomLeftRadius);
			processor(object.BorderBottomRightRadius);

			processor(object.FontFamily);
			processor(object.FontSize);
			processor(object.HorizontalTextAlignment);
			processor(object.VerticalTextAlignment);
			processor(object.WordWrap);

			processor(object.OverridenProperties);
		}
	};

	template<>
	struct RTTIPlainType<GUIStyleSheetStyle> : RTTIPlainTypeHelper<GUIStyleSheetStyle, TID_GUIStyleSheetStyle, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetStyle& object, Processor& processor, u8 version)
		{
			processor(object.Normal);
			processor(object.Focus);
			processor(object.Hover);
			processor(object.Active);
			processor(object.Disabled);
			processor(object.Checked);
		}
	};

	class B3D_EXPORT GUIStyleSheetRTTI : public RTTIType<GUIStyleSheet, Resource, GUIStyleSheetRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN_MAP(mElementStyles, 0, Normal.Selector)
			B3D_RTTI_MEMBER_PLAIN_MAP(mIdStyles, 1, Normal.Selector)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "GUIStyleSheet";
			return name;
		}

		u32 GetRttiId() override 
		{
			return TID_GUIStyleSheet;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return GUIStyleSheet::CreateShared();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
