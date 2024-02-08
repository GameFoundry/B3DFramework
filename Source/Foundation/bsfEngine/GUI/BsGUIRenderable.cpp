//************************************ bs::framework - Copyright 2023 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIRenderable.h"
#include "BsGUIHelper.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIManager.h"
#include "Resources/BsBuiltinResources.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIRenderableRTTI : public RTTIType<GUIRenderable, GUIElement, GUIRenderableRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIRenderable";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIRenderable; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

/** @} */
/** @endcond */

const GUIStyleSheetRuleInformation GUIStyleSheetRuleInformation::kInvalid("Invalid");
const Color GUIRenderable::kDisabledColor = Color(0.5f, 0.5f, 0.5f, 1.0f);

GUIRenderable::GUIRenderable(String styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElement(sizeConstraints), mStyle(&GUISkin::DefaultStyle), mStyleClass(std::move(styleClass))
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

GUIRenderable::GUIRenderable(const char* styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElement(sizeConstraints), mStyle(&GUISkin::DefaultStyle), mStyleClass(styleClass ? styleClass : StringUtil::kBlank)
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

bool GUIRenderable::IsUsingStyleSheets() const
{
	if(mStyleSheetRuleInformation.StateRulesets == nullptr || mStyleSheetRuleInformation.StateRulesets->StyleSheets.Empty())
		return false;

	for(const auto& entry : mStyleSheetRuleInformation.StateRulesets->StyleSheets)
	{
		if(!entry.RulesetIndices.Empty())
			return true;
	}

	return false;
}

void GUIRenderable::UpdateRenderElements()
{
	UpdateClippedBounds();
}

void GUIRenderable::GetRenderElementVertexAndIndexData(u32 renderElementIndex, u32 vertexOffset, u32 indexOffset, DataRange& outPositions, DataRange& outUVs, DataRange& outIndices) const
{
	const GUIRenderElement& renderElement = mRenderElements[renderElementIndex];

	const Vector2I guiElementOffset(mLayoutData.Area.X, mLayoutData.Area.Y);
	const Rect2 guiElementClipRectangle = (Rect2)mLayoutData.GetLocalClipRect();

	// Build the render element bounds to use for clipping
	Rect2 renderElementClipRectangle = renderElement.ClipRectangle; // In space relative to parent GUI element

	// Clip by the elements clip rectangle
	renderElementClipRectangle.Clip(guiElementClipRectangle);

	// Move the bounds into space relative to the content bounds (as the offset gets applied after clipping).
	renderElementClipRectangle.X -= renderElement.Offset.X;
	renderElementClipRectangle.Y -= renderElement.Offset.Y;

	const Vector2 renderElementOffset =
		Vector2((float)guiElementOffset.X, (float)guiElementOffset.Y) +
		renderElement.Offset;

	renderElement.GetVertexAndIndexData(vertexOffset, indexOffset, renderElementOffset, renderElementClipRectangle, true, outPositions, outUVs, outIndices);
}

void GUIRenderable::UpdateClippedBounds()
{
	mClippedBounds = mLayoutData.Area;
	mClippedBounds.Clip(mLayoutData.ClipRect);
}

void GUIRenderable::SetStyleSheetClass(const String& styleClass)
{
	mStyleClass = styleClass;
	RefreshStyle();
}

void GUIRenderable::SetTint(const Color& color)
{
	mColor = color;

	MarkContentAsDirty();
}

void GUIRenderable::SetElementDepth(u8 depth)
{
	mLayoutData.Depth = depth | (mLayoutData.Depth & 0xFFFFFF00);
	MarkMeshAsDirty();
}

u8 GUIRenderable::GetElementDepth() const
{
	return mLayoutData.Depth & 0xFF;
}

void GUIRenderable::SetLayoutData(const GUILayoutData& data)
{
	// Preserve element depth as that is not controlled by layout but is stored
	// there only for convenience
	u8 elemDepth = GetElementDepth();
	GUIElement::SetLayoutData(data);
	SetElementDepth(elemDepth);

	UpdateClippedBounds();
}

void GUIRenderable::ChangeParentWidget(GUIWidget* widget)
{
	if(IsPendingDestroy())
		return;

	const bool widgetChanged = mParentWidget != widget;
	GUIElement::ChangeParentWidget(widget);

	if(widgetChanged)
		RefreshStyle();
}

const RectOffset& GUIRenderable::GetMargins() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset)
		return mStyleSheetRuleInformation.CurrentStateRuleset->Rules.Margins;
	else if(mStyle != nullptr)
		return mStyle->Padding; // Note: Old GUI style has the meaning of padding/margins swapped
	else
	{
		static RectOffset margins;
		return margins;
	}
}

const RectOffset& GUIRenderable::GetPadding() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset)
		return mStyleSheetRuleInformation.CurrentStateRuleset->Rules.Padding;
	else if(mStyle != nullptr)
		return mStyle->Margins; // Note: Old GUI style has the meaning of padding/margins swapped
	else
	{
		static RectOffset padding;
		return padding;
	}
}

void GUIRenderable::ResetDimensions()
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints = GUISizeConstraints::Create();
	mSizeConstraints.UpdateWithStyle(mStyle);

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

Rect2I GUIRenderable::GetCachedContentBounds() const
{
	Rect2I contentArea = GetCachedContentBoundsInElementSpace();

	const Rect2I& cachedBounds = GetCachedBounds();
	contentArea.X += cachedBounds.X;
	contentArea.Y += cachedBounds.Y;

	return contentArea;
}

Rect2I GUIRenderable::GetCachedContentBoundsInElementSpace() const
{
	const Rect2I& cachedBounds = GetCachedBounds();
	const Size2UI layoutSize(cachedBounds.Width, cachedBounds.Height);

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		return GUIHelper::CalculateContentArea(layoutSize, styleSheetRules);
	}

	return GUIHelper::CalculateContentArea(layoutSize, *mStyle);
}

Rect2I GUIRenderable::GetCachedClippedContentBoundsInContentSpace() const
{
	const Rect2I localContentBounds = GetCachedContentBoundsInElementSpace();

	// Transform into element space so we can clip it using the element clip rectangle
	Rect2I contentClipRect = localContentBounds;
	contentClipRect.Clip(mLayoutData.GetLocalClipRect());

	// Transform into content sprite space
	contentClipRect.X -= localContentBounds.X;
	contentClipRect.Y -= localContentBounds.Y;

	return contentClipRect;
}

bool GUIRenderable::IsInBounds(const Vector2I& position) const
{
	return GetCachedClippedBounds().Contains(position);
}

Vector2I GUIRenderable::GetContentOffsetInElementSpace() const
{
	const RectOffset& padding = GetPadding();
	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		return Vector2I(
			padding.Left + styleSheetRules.BorderLeft.GetVisibleWidth(),
			padding.Top + styleSheetRules.BorderTop.GetVisibleWidth());
	}
	else
	{
		return Vector2I(
			padding.Left + mStyle->ContentOffset.Left,
			padding.Top + mStyle->ContentOffset.Top);
	}
}

Color GUIRenderable::GetTint() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr) // With style sheets, disabled color is controlled via a separate rule, rather than being hardcoded
		return mColor;
	else
	{
		if(!IsDisabled())
			return mColor;

		return mColor * kDisabledColor;
	}
}

u32 GUIRenderable::RegisterPseudoElement(const char* name)
{
	if(!B3D_ENSURE(name != nullptr))
		return ~0u;

	const u32 pseudoElementIndex = (u32)mPseudoElementStyleSheetRules.Size();
	mPseudoElementStyleSheetRules.Add(GUIStyleSheetRuleInformation(name));
	RefreshStyle();

	return pseudoElementIndex;
}

const GUIStyleSheetRuleInformation& GUIRenderable::GetPseudoElementStyleSheetRuleInformation(u32 pseudoElementIndex) const
{
	if(!B3D_ENSURE(pseudoElementIndex < (u32)mPseudoElementStyleSheetRules.Size()))
		return GUIStyleSheetRuleInformation::kInvalid;

	return mPseudoElementStyleSheetRules[pseudoElementIndex];
}

void GUIRenderable::RefreshStyle()
{
	const bool isUsingStyleSheets = GetStyleSheetElement() != nullptr;
	if(isUsingStyleSheets)
	{
		const GUIWidget* parentWidget = GetParentWidget();
		const GUIStyleSheetCascade& styleSheetCascade = parentWidget != nullptr ? parentWidget->GetStyleSheetCascade() : GUIStyleSheetCascade::kEmpty;

		if(styleSheetCascade.HasRulesetForClass(GetStyleSheetClass(), GetStyleSheetElement()))
		{
			SPtr<const GUIStyleSheetStateRulesets> newStateRulesets = styleSheetCascade.BuildStateRulesets(*this);

			if(!newStateRulesets)
				newStateRulesets = GUIStyleSheetStateRulesets::kDefault;

			bool anyRuleChanged = false;
			if(newStateRulesets != mStyleSheetRuleInformation.StateRulesets)
			{
				mStyleSheetRuleInformation.StateRulesets = newStateRulesets;
				mStyleSheetRuleInformation.CurrentStateRuleset = nullptr;

				if(IsUsingStyleSheets())
				{
					const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

					mStyleSheetRuleInformation.CurrentStateRuleset = mStyleSheetRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags);
					mSizeConstraints.UpdateWithStyleSheetRule(mStyleSheetRuleInformation.CurrentStateRuleset->Rules);

					const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();
					if(isFixedBefore != isFixedAfter)
						RefreshLayoutUpdateParentsForChildren();

					anyRuleChanged = true;
				}
			}

			if(IsUsingStyleSheets())
			{
				const GUIStyleSheetRules* inheritedRules = mStyleSheetRuleInformation.CurrentStateRuleset != nullptr ? &mStyleSheetRuleInformation.CurrentStateRuleset->Rules : nullptr;
				for(auto& pseudoElementRuleInformation : mPseudoElementStyleSheetRules)
				{
					SPtr<const GUIStyleSheetStateRulesets> newPseudoElementStateRulesets = styleSheetCascade.BuildStateRulesets(*this, pseudoElementRuleInformation.PseudoElementName);

					if(!newPseudoElementStateRulesets)
						newPseudoElementStateRulesets = GUIStyleSheetStateRulesets::kDefault;

					pseudoElementRuleInformation.StateRulesets = newPseudoElementStateRulesets;
					pseudoElementRuleInformation.CurrentStateRuleset = pseudoElementRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags, inheritedRules);
				}

				if(anyRuleChanged)
				{
					NotifyStyleChanged();
					MarkLayoutAsDirty();
				}
			}
		}
		else
		{
			mStyleSheetRuleInformation.StateRulesets = nullptr;
			mStyleSheetRuleInformation.CurrentStateRuleset = nullptr;

			for(auto& pseudoElementRuleInformation : mPseudoElementStyleSheetRules)
			{
				pseudoElementRuleInformation.StateRulesets = nullptr;
				pseudoElementRuleInformation.CurrentStateRuleset = nullptr;
			}
		}
	}

	// DEPRECATED
	if(!IsUsingStyleSheets())
	{
		const GUIElementStyle* newStyle = nullptr;
		if(GetParentWidget() != nullptr && !mStyleClass.empty())
			newStyle = GetParentWidget()->GetSkin().GetStyle(mStyleClass);
		else
			newStyle = &GUISkin::DefaultStyle;

		if(newStyle != mStyle)
		{
			mStyle = newStyle;

			const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

			mSizeConstraints.UpdateWithStyle(mStyle);

			const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();
			if(isFixedBefore != isFixedAfter)
				RefreshLayoutUpdateParentsForChildren();

			NotifyStyleChanged();
			MarkLayoutAsDirty();
		}
	}
}

RTTITypeBase* GUIRenderable::GetRttiStatic()
{
	return GUIRenderableRTTI::Instance();
}

RTTITypeBase* GUIRenderable::GetRtti() const
{
	return GetRttiStatic();
}

const String& GUIRenderable::GetSubStyleName(const String& subStyleTypeName) const
{
	auto iterFind = mStyle->SubStyles.find(subStyleTypeName);

	if(iterFind != mStyle->SubStyles.end())
		return iterFind->second;
	else
		return StringUtil::kBlank;
}
