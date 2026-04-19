//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/B3DGUIRenderable.h"
#include "B3DGUIUtility.h"
#include "GUI/B3DGUIWidget.h"
#include "GUI/B3DGUIManager.h"
#include "Resources/B3DBuiltinResources.h"
#include "StyleSheet/B3DGUIStyleSheet.h"

using namespace b3d;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace b3d
{
	class B3D_EXPORT GUIRenderableRTTI : public TRTTIType<GUIRenderable, GUIElement, GUIRenderableRTTI>
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
} // namespace b3d

/** @} */
/** @endcond */

const GUIStyleSheetRuleInformation GUIStyleSheetRuleInformation::kInvalid("Invalid");
const Color GUIRenderable::kDisabledColor = Color(0.5f, 0.5f, 0.5f, 1.0f);

GUIRenderable::GUIRenderable(String styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElement(sizeConstraints), mStyleClass(std::move(styleClass))
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

GUIRenderable::GUIRenderable(const char* styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElement(sizeConstraints), mStyleClass(styleClass ? styleClass : StringUtility::kBlank)
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

void GUIRenderable::GetRenderElementVertexAndIndexData(u32 renderElementIndex, u32 vertexOffset, u32 indexOffset, DataRange& outPositions, DataRange& outUVs, DataRange& outIndices) const
{
	const GUIRenderElement& renderElement = mRenderElements[renderElementIndex];

	const GUIPhysicalPoint guiElementOffset = mAbsolutePosition;
	const Area2 guiElementClipRectangle = GetClippedAreaTranslatedRelativeToParent().To<float>();

	// Build the render element bounds to use for clipping
	Area2 renderElementClipRectangle = renderElement.ClipRectangle; // In space relative to parent GUI element

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
	else
	{
		static RectOffset padding;
		return padding;
	}
}

void GUIRenderable::ResetSizeConstraints()
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints = GUISizeConstraints::Create();

	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
		mSizeConstraints.UpdateWithStyleSheetRule(mStyleSheetRuleInformation.CurrentStateRuleset->Rules);

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

GUIPhysicalArea GUIRenderable::GetAbsoluteContentBounds() const
{
	const GUIPhysicalPoint absolutePosition = GetAbsolutePosition();

	GUIPhysicalArea absoluteContentArea = GetScaledContentBounds();
	absoluteContentArea.X += absolutePosition.X;
	absoluteContentArea.Y += absolutePosition.Y;

	return absoluteContentArea;
}

GUILogicalArea GUIRenderable::GetContentBounds() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;
		return GUIUtility::RemovePaddingAndBorder(mLayoutData.Size, styleSheetRules);
	}

	return GUILogicalArea(GUILogicalPoint(0, 0), mLayoutData.Size);
}

GUIPhysicalArea GUIRenderable::GetScaledContentBounds() const
{
	float parentAbsoluteScale = 1.0f;
	if(mParent != nullptr)
		parentAbsoluteScale = mParent->GetAbsoluteScale();

	GUIPhysicalArea contentArea = GetContentBounds().To<GUIPhysicalUnit>();
	contentArea.X = Math::RoundToI32((float)contentArea.X * parentAbsoluteScale);
	contentArea.Y = Math::RoundToI32((float)contentArea.Y * parentAbsoluteScale);
	contentArea.Width = Math::RoundToI32((float)contentArea.Width * GetAbsoluteScale());
	contentArea.Height = Math::RoundToI32((float)contentArea.Height * GetAbsoluteScale());
	
	return contentArea;
}

GUILogicalPoint GUIRenderable::GetContentOffset() const
{
	if(mStyleSheetRuleInformation.CurrentStateRuleset != nullptr)
	{
		const GUIStyleSheetRules& styleSheetRules = mStyleSheetRuleInformation.CurrentStateRuleset->Rules;

		const RectOffset& padding = GetPadding();
		return GUILogicalPoint(
			padding.Left + styleSheetRules.BorderLeft.GetVisibleWidth(),
			padding.Top + styleSheetRules.BorderTop.GetVisibleWidth());
	}

	return GUILogicalPoint(0, 0);
}

GUIPhysicalPoint GUIRenderable::GetScaledContentOffset() const
{
	return (GetContentOffset().To<float>() * GetAbsoluteScale()).To<GUIPhysicalUnit>();
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
	bool anyRuleChanged = false;
	if(GetStyleSheetElement() != nullptr)
	{
		const GUIWidget* parentWidget = GetParentWidget();
		const GUIStyleSheetCascade& styleSheetCascade = parentWidget != nullptr ? parentWidget->GetStyleSheetCascade() : GUIStyleSheetCascade::kEmpty;

		SPtr<const GUIStyleSheetStateRulesets> newStateRulesets = styleSheetCascade.BuildStateRulesets(*this);

		if(!newStateRulesets)
			newStateRulesets = GUIStyleSheetStateRulesets::kDefault;

		if(newStateRulesets != mStyleSheetRuleInformation.StateRulesets)
		{
			mStyleSheetRuleInformation.StateRulesets = newStateRulesets;
			mStyleSheetRuleInformation.CurrentStateRuleset = nullptr;

			const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

			mStyleSheetRuleInformation.CurrentStateRuleset = mStyleSheetRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags);
			mSizeConstraints.UpdateWithStyleSheetRule(mStyleSheetRuleInformation.CurrentStateRuleset->Rules);

			const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();
			if(isFixedBefore != isFixedAfter)
				RefreshLayoutUpdateParentsForChildren();

			anyRuleChanged = true;
		}

		const GUIStyleSheetRules* inheritedRules = mStyleSheetRuleInformation.CurrentStateRuleset != nullptr ? &mStyleSheetRuleInformation.CurrentStateRuleset->Rules : nullptr;
		for(auto& pseudoElementRuleInformation : mPseudoElementStyleSheetRules)
		{
			SPtr<const GUIStyleSheetStateRulesets> newPseudoElementStateRulesets = styleSheetCascade.BuildStateRulesets(*this, pseudoElementRuleInformation.PseudoElementName);

			if(!newPseudoElementStateRulesets)
				newPseudoElementStateRulesets = GUIStyleSheetStateRulesets::kDefault;

			if(pseudoElementRuleInformation.StateRulesets != newPseudoElementStateRulesets)
			{
				pseudoElementRuleInformation.StateRulesets = newPseudoElementStateRulesets;
				pseudoElementRuleInformation.CurrentStateRuleset = pseudoElementRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags, inheritedRules);
				anyRuleChanged = true;
			}
		}
	}
	else
	{
		if(mStyleSheetRuleInformation.StateRulesets != nullptr)
		{
			mStyleSheetRuleInformation.StateRulesets = nullptr;
			mStyleSheetRuleInformation.CurrentStateRuleset = nullptr;

			anyRuleChanged = true;
		}

		for(auto& pseudoElementRuleInformation : mPseudoElementStyleSheetRules)
		{
			if(pseudoElementRuleInformation.StateRulesets != nullptr)
			{
				pseudoElementRuleInformation.StateRulesets = nullptr;
				pseudoElementRuleInformation.CurrentStateRuleset = nullptr;

				anyRuleChanged = true;
			}
		}
	}

	if(anyRuleChanged)
	{
		NotifyStyleChanged();
		MarkLayoutAsDirty();
	}
}

RTTIType* GUIRenderable::GetRttiStatic()
{
	return GUIRenderableRTTI::Instance();
}

RTTIType* GUIRenderable::GetRtti() const
{
	return GetRttiStatic();
}
