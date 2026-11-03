//************************************* B3D Framework - Copyright 2026 Marko Pintera *************************************//
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
		const String& GetRttiName() override
		{
			static String name = "GUIRenderable";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIRenderable; }

		TShared<IReflectable> NewRttiObject() override { return nullptr; }
	};
} // namespace b3d

/** @} */
/** @endcond */

const GUIStyleSheetRuleInformation GUIStyleSheetRuleInformation::kInvalid("Invalid");
const Color GUIRenderable::kDisabledColor = Color(0.5f, 0.5f, 0.5f, 1.0f);

GUIRenderable::GUIRenderable(String styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElement(sizeConstraints)
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
	if(!styleClass.empty())
		mStyleClasses.Add(std::move(styleClass));
}

GUIRenderable::GUIRenderable(const char* styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElement(sizeConstraints)
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
	if(styleClass != nullptr && styleClass[0] != '\0')
		mStyleClasses.Add(String(styleClass));
}

const String& GUIRenderable::GetStyleSheetClass() const
{
	return mStyleClasses.Empty() ? StringUtility::kBlank : mStyleClasses[0];
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
	TInlineArray<String, 1> newStyleClasses;
	if(!styleClass.empty())
		newStyleClasses.Add(styleClass);

	SetStyleClasses(std::move(newStyleClasses));
}

bool GUIRenderable::HasStyleClass(const String& styleClass) const
{
	for(const auto& entry : mStyleClasses)
	{
		if(entry == styleClass)
			return true;
	}

	return false;
}

void GUIRenderable::AddStyleClass(const String& styleClass)
{
	if(styleClass.empty() || HasStyleClass(styleClass))
		return;

	const bool wasLayoutOnly = IsLayoutOnly();

	mStyleClasses.Add(styleClass);
	RefreshStyleAndRegistration(wasLayoutOnly);
}

void GUIRenderable::RemoveStyleClass(const String& styleClass)
{
	for(auto it = mStyleClasses.begin(); it != mStyleClasses.end(); ++it)
	{
		if(*it != styleClass)
			continue;

		const bool wasLayoutOnly = IsLayoutOnly();

		mStyleClasses.Erase(it);
		RefreshStyleAndRegistration(wasLayoutOnly);
		return;
	}
}

void GUIRenderable::ToggleStyleClass(const String& styleClass, bool enabled)
{
	if(enabled)
		AddStyleClass(styleClass);
	else
		RemoveStyleClass(styleClass);
}

void GUIRenderable::SetStyleClasses(TInlineArray<String, 1> styleClasses)
{
	if(styleClasses == mStyleClasses)
		return;

	const bool wasLayoutOnly = IsLayoutOnly();

	mStyleClasses = std::move(styleClasses);
	RefreshStyleAndRegistration(wasLayoutOnly);
}

void GUIRenderable::ClearStyleClasses()
{
	if(mStyleClasses.Empty())
		return;

	const bool wasLayoutOnly = IsLayoutOnly();

	mStyleClasses.Clear();
	RefreshStyleAndRegistration(wasLayoutOnly);
}

void GUIRenderable::RefreshStyleAndRegistration(bool wasLayoutOnly)
{
	// Gaining the first class or losing the last one can turn a layout-only element into a drawn one, or back
	if(wasLayoutOnly != IsLayoutOnly())
	{
		if(mParentWidget != nullptr && !IsPendingDestroy())
			RefreshWidgetRegistration();

		// Selectors on the elements below now match through this element, or stop at it, so they all need restyling even if
		// the key they build on stays the same
		mStyleKeyRevision = 0;
	}

	RefreshStyleRecursive();
}

void GUIRenderable::RefreshWidgetRegistration()
{
	mParentWidget->UnregisterElement(this);
	mParentWidget->RegisterElement(this);
}

u64 GUIRenderable::GetStyleKey() const
{
	EnsureStyleKeys();
	return mStyleKey;
}

GUIElementStates GUIRenderable::GetStatesAffectingDescendants() const
{
	EnsureStyleKeys();
	return mStatesAffectingDescendants;
}

const GUIStyleSheetCascade& GUIRenderable::GetActiveStyleSheetCascade() const
{
	const GUIWidget* const parentWidget = GetParentWidget();
	return parentWidget != nullptr ? parentWidget->GetStyleSheetCascade() : GUIStyleSheetCascade::kEmpty;
}

const GUIRenderable* GUIRenderable::GetStyleParent() const
{
	for(const GUIElement* parent = GetParent(); parent != nullptr; parent = parent->GetParent())
	{
		const GUIRenderable* const parentRenderable = B3DRTTICast<const GUIRenderable>(parent);
		if(parentRenderable != nullptr && !parentRenderable->IsLayoutOnly())
			return parentRenderable;
	}

	return nullptr;
}

void GUIRenderable::EnsureStyleKeys() const
{
	if(mStyleKeyRevision != GetActiveStyleSheetCascade().GetRevision())
		UpdateStyleKeys();
}

void GUIRenderable::UpdateStyleKeys() const
{
	const GUIStyleSheetCascade& styleSheetCascade = GetActiveStyleSheetCascade();

	const GUIRenderable* const parentRenderable = GetStyleParent();

	u64 ancestorStyleKey = 0;
	if(parentRenderable != nullptr)
	{
		parentRenderable->EnsureStyleKeys();
		ancestorStyleKey = parentRenderable->mDescendantStyleKey;
	}

	// Selectors cannot observe an element that is layout only, so it neither matches anything nor extends the key
	if(IsLayoutOnly())
	{
		mStyleKey = ancestorStyleKey;
		mDescendantStyleKey = ancestorStyleKey;
		mStatesAffectingDescendants = GUIElementState::Normal;
		mHasParentOfMatch = false;
		mStyleKeyRevision = styleSheetCascade.GetRevision();

		return;
	}

	// Every value is preceded by the part it came from, so that for example a class and an id of the same name, or a class
	// on a parent and the type of its child, never produce the same key
	mStyleKey = ancestorStyleKey;
	B3DCombineHash(mStyleKey, GUIStyleKeyPart::ElementType);
	B3DCombineHash(mStyleKey, GetStyleSheetElement());

	// Selectors only care which classes are present, so the order the classes were added in must not affect the key
	TInlineArray<u64, 4> styleClassHashes;
	for(const auto& styleClass : mStyleClasses)
	{
		u64 styleClassHash = 0;
		B3DCombineHash(styleClassHash, styleClass);
		styleClassHashes.Add(styleClassHash);
	}

	std::sort(styleClassHashes.begin(), styleClassHashes.end());
	for(u64 styleClassHash : styleClassHashes)
	{
		B3DCombineHash(mStyleKey, GUIStyleKeyPart::Class);
		B3DCombineHash(mStyleKey, styleClassHash);
	}

	const String& styleId = GetStyleSheetId();
	if(!styleId.empty())
	{
		B3DCombineHash(mStyleKey, GUIStyleKeyPart::Id);
		B3DCombineHash(mStyleKey, styleId);
	}

	// Descendants can only observe this element through the ancestor parts of the selectors, so that is all they get. The
	// element's own state is resolved per state after the lookup, so it is only part of what descendants see.
	const GUIStyleSheetAncestorMatch ancestorMatch = styleSheetCascade.MatchAncestorSelectors(*this);
	mStatesAffectingDescendants = ancestorMatch.StatesAffectingDescendants;
	mHasParentOfMatch = ancestorMatch.HasParentOfMatch;

	// Below a parent constrained by a `>` combinator, this element extends the key even when nothing matches it, so its
	// descendants do not end up sharing a key with the parent's direct children
	mDescendantStyleKey = ancestorStyleKey;
	if(ancestorMatch.IsObservableByDescendants || (parentRenderable != nullptr && parentRenderable->mHasParentOfMatch))
	{
		B3DCombineHash(mDescendantStyleKey, GUIStyleKeyPart::Ancestor);
		B3DCombineHash(mDescendantStyleKey, ancestorMatch.MatchingSelectorsHash);
	}

	mStyleKeyRevision = styleSheetCascade.GetRevision();
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

	// The new widget may share the style sheet cascade, in which case the cascade revision alone would not reveal that the
	// ancestors changed. Every element below goes through here as well.
	const bool widgetChanged = mParentWidget != widget;
	if(widgetChanged)
		mStyleKeyRevision = 0;

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
		const RectOffset border = styleSheetRules.GetBorderInsets();
		return GUILogicalPoint(padding.Left + border.Left, padding.Top + border.Top);
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

void GUIRenderable::RefreshStyleRecursive()
{
	const bool hadStyleKeys = mStyleKeyRevision == GetActiveStyleSheetCascade().GetRevision();
	const u64 previousDescendantStyleKey = mDescendantStyleKey;

	UpdateStyleKeys();
	RefreshStyle();

	// Elements below build their style keys on the descendant style key, so nothing they match on changed if it did not
	if(!hadStyleKeys || mDescendantStyleKey != previousDescendantStyleKey)
		Super::RefreshStyleRecursive();
}

void GUIRenderable::RefreshStyle()
{
	bool anyRuleChanged = false;

	const GUIStyleSheetCascade& styleSheetCascade = GetActiveStyleSheetCascade();

	// Elements that are layout only never consult the style sheet, and only fall back to the default rules if they were
	// styled before
	TShared<const GUIStyleSheetStateRulesets> newStateRulesets;
	if(!IsLayoutOnly())
		newStateRulesets = styleSheetCascade.BuildStateRulesets(*this);
	else if(mStyleSheetRuleInformation.StateRulesets == nullptr)
		return;

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
		TShared<const GUIStyleSheetStateRulesets> newPseudoElementStateRulesets = styleSheetCascade.BuildStateRulesets(*this, pseudoElementRuleInformation.PseudoElementName);

		if(!newPseudoElementStateRulesets)
			newPseudoElementStateRulesets = GUIStyleSheetStateRulesets::kDefault;

		if(pseudoElementRuleInformation.StateRulesets != newPseudoElementStateRulesets)
		{
			pseudoElementRuleInformation.StateRulesets = newPseudoElementStateRulesets;
			pseudoElementRuleInformation.CurrentStateRuleset = pseudoElementRuleInformation.StateRulesets->BuildStateRuleset(mStateFlags, inheritedRules);
			anyRuleChanged = true;
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
