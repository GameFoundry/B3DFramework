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
#include "Private/RTTI/BsFontRTTI.h"
#include "Private/RTTI/BsTextureRTTI.h"
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

	class B3D_EXPORT GUIStyleSheetRuleRTTI : public RTTIType<GUIStyleSheetRules, IReflectable, GUIStyleSheetRuleRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(Margins, 0)
			B3D_RTTI_MEMBER_PLAIN(Padding, 1)

			B3D_RTTI_MEMBER_PLAIN(Size, 2)
			B3D_RTTI_MEMBER_PLAIN(MinimumSize, 3)
			B3D_RTTI_MEMBER_PLAIN(MaximumSize, 4)

			B3D_RTTI_MEMBER_PLAIN(BackgroundColor, 5)
			B3D_RTTI_MEMBER_PLAIN(Color, 6)
			B3D_RTTI_MEMBER_PLAIN(Opacity, 7)

			B3D_RTTI_MEMBER_REFL(BackgroundImage, 8)
			B3D_RTTI_MEMBER_PLAIN(Visibility, 9)

			B3D_RTTI_MEMBER_PLAIN(BorderLeft, 10)
			B3D_RTTI_MEMBER_PLAIN(BorderRight, 11)
			B3D_RTTI_MEMBER_PLAIN(BorderTop, 12)
			B3D_RTTI_MEMBER_PLAIN(BorderBottom, 13)

			B3D_RTTI_MEMBER_PLAIN(BorderTopLeftRadius, 14)
			B3D_RTTI_MEMBER_PLAIN(BorderTopRightRadius, 15)
			B3D_RTTI_MEMBER_PLAIN(BorderBottomLeftRadius, 16)
			B3D_RTTI_MEMBER_PLAIN(BorderBottomRightRadius, 17)

			B3D_RTTI_MEMBER_REFL(Font, 18)
			B3D_RTTI_MEMBER_PLAIN(FontSize, 19)
			B3D_RTTI_MEMBER_PLAIN(HorizontalTextAlignment, 20)
			B3D_RTTI_MEMBER_PLAIN(VerticalTextAlignment, 21)
			B3D_RTTI_MEMBER_PLAIN(WordWrap, 22)

			B3D_RTTI_MEMBER_PLAIN(OverridenProperties, 23)

			B3D_RTTI_MEMBER_PLAIN(PseudoClass, 24)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "GUIStyleSheetRule";
			return name;
		}

		u32 GetRttiId() override 
		{
			return TID_GUIStyleSheetRule;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<GUIStyleSheetRules>();
		}
	};

	class B3D_EXPORT GUIStyleSheetRulesetRTTI : public RTTIType<GUIStyleSheetRuleset, IReflectable, GUIStyleSheetRulesetRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_PLAIN(SelectorList, 0)
			B3D_RTTI_MEMBER_REFL(Rules, 1)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "GUIStyleSheetRuleset";
			return name;
		}

		u32 GetRttiId() override 
		{
			return TID_GUIStyleSheetRuleset;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<GUIStyleSheetRuleset>();
		}
	};

	class B3D_EXPORT GUIStyleSheetRTTI : public RTTIType<GUIStyleSheet, Resource, GUIStyleSheetRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_REFL_ARRAY(mRulesets, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnDeserializationEnded(IReflectable* obj, SerializationContext* context) override
		{
			GUIStyleSheet* const styleSheet = static_cast<GUIStyleSheet*>(obj);
			styleSheet->Initialize();
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
			return GUIStyleSheet::CreateUninitialized();
		}
	};

	/** @} */
	/** @endcond */
} // namespace bs
