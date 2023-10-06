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
#include "RTTI/BsTArrayRTTI.h"
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
	struct RTTIPlainType<GUIStyleSheetSelector> : RTTIPlainTypeHelper<GUIStyleSheetSelector, TID_GUIStyleSheetSelector, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetSelector& object, Processor& processor, u8 version)
		{
			processor(object.Name);
			processor(object.SelectorType);
			processor(object.CombinatorType);
		}
	};

	template<>
	struct RTTIPlainType<GUIStyleSheetSelectorList> : RTTIPlainTypeHelper<GUIStyleSheetSelectorList, TID_GUIStyleSheetSelectorList, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetSelectorList& object, Processor& processor, u8 version)
		{
			processor(object.Selectors);
		}
	};

	template<>
	struct RTTIPlainType<GUIStyleSheetStateRule> : RTTIPlainTypeHelper<GUIStyleSheetStateRule, TID_GUIStyleSheetStyleState, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetStateRule& object, Processor& processor, u8 version)
		{
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
	struct RTTIPlainType<GUIStyleSheetRule> : RTTIPlainTypeHelper<GUIStyleSheetRule, TID_GUIStyleSheetStyle, 0>
	{
		template <class Processor>
		static void RTTIEnumerateFields(GUIStyleSheetRule& object, Processor& processor, u8 version)
		{
			processor(object.SelectorList);
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
			B3D_RTTI_MEMBER_PLAIN_ARRAY(mRules, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			GUIStyleSheet* const styleSheet = static_cast<GUIStyleSheet*>(obj);
			styleSheet->RebuildCache();
		}

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
