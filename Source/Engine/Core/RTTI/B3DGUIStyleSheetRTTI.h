//************************************ B3D Framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Reflection/B3DRTTIType.h"
#include "RTTI/B3DStringRTTI.h"
#include "RTTI/B3DBitfieldRTTI.h"
#include "RTTI/B3DRectOffsetRTTI.h"
#include "RTTI/B3DMathRTTI.h"
#include "RTTI/B3DColorRTTI.h"
#include "RTTI/B3DTArrayRTTI.h"
#include "RTTI/B3DStdRTTI.h"
#include "RTTI/B3DFontRTTI.h"
#include "RTTI/B3DTextureRTTI.h"
#include "GUI/StyleSheet/B3DGUIStyleSheet.h"

namespace b3d
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

	class B3D_EXPORT GUIStyleSheetRuleRTTI : public TRTTIType<GUIStyleSheetRules, IReflectable, GUIStyleSheetRuleRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(Margins, 0)
			B3D_RTTI_MEMBER(Padding, 1)

			B3D_RTTI_MEMBER(Size, 2)
			B3D_RTTI_MEMBER(MinimumSize, 3)
			B3D_RTTI_MEMBER(MaximumSize, 4)

			B3D_RTTI_MEMBER(BackgroundColor, 5)
			B3D_RTTI_MEMBER(Color, 6)
			B3D_RTTI_MEMBER(Opacity, 7)

			B3D_RTTI_MEMBER(BackgroundImage, 8)
			B3D_RTTI_MEMBER(Visibility, 9)

			B3D_RTTI_MEMBER(BorderLeft, 10)
			B3D_RTTI_MEMBER(BorderRight, 11)
			B3D_RTTI_MEMBER(BorderTop, 12)
			B3D_RTTI_MEMBER(BorderBottom, 13)

			B3D_RTTI_MEMBER(BorderTopLeftRadius, 14)
			B3D_RTTI_MEMBER(BorderTopRightRadius, 15)
			B3D_RTTI_MEMBER(BorderBottomLeftRadius, 16)
			B3D_RTTI_MEMBER(BorderBottomRightRadius, 17)

			B3D_RTTI_MEMBER(Font, 18)
			B3D_RTTI_MEMBER(FontSize, 19)
			B3D_RTTI_MEMBER(HorizontalTextAlignment, 20)
			B3D_RTTI_MEMBER(VerticalTextAlignment, 21)
			B3D_RTTI_MEMBER(WordWrap, 22)

			B3D_RTTI_MEMBER(OverridenProperties, 23)

			B3D_RTTI_MEMBER(PseudoClass, 24)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "GUIStyleSheetRule";
			return name;
		}

		u32 GetRttiId() const override 
		{
			return TID_GUIStyleSheetRule;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<GUIStyleSheetRules>();
		}
	};

	class B3D_EXPORT GUIStyleSheetRulesetRTTI : public TRTTIType<GUIStyleSheetRuleset, IReflectable, GUIStyleSheetRulesetRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER(SelectorList, 0)
			B3D_RTTI_MEMBER(Rules, 1)
		B3D_RTTI_END_MEMBERS
	public:
		const String& GetRttiName() override
		{
			static String name = "GUIStyleSheetRuleset";
			return name;
		}

		u32 GetRttiId() const override 
		{
			return TID_GUIStyleSheetRuleset;
		}

		SPtr<IReflectable> NewRttiObject() override
		{
			return B3DMakeShared<GUIStyleSheetRuleset>();
		}
	};

	class B3D_EXPORT GUIStyleSheetRTTI : public TRTTIType<GUIStyleSheet, Resource, GUIStyleSheetRTTI>
	{
	private:
		B3D_RTTI_BEGIN_MEMBERS
			B3D_RTTI_MEMBER_CONTAINER(mRulesets, 0)
		B3D_RTTI_END_MEMBERS
	public:
		void OnOperationEnded(GUIStyleSheet& object, RTTIOperationTypeFlags operationType, RTTIOperationContext& context) override
		{
			if(operationType.IsSet(RTTIOperationType::WriteBit) && !operationType.IsSet(RTTIOperationType::PreExistingObjectBit))
				object.Initialize();
		}

		const String& GetRttiName() override
		{
			static String name = "GUIStyleSheet";
			return name;
		}

		u32 GetRttiId() const override 
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
} // namespace b3d
