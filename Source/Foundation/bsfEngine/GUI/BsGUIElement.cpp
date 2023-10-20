//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIManager.h"
#include "BsGUINavGroup.h"
#include "StyleSheet/BsGUIStyleSheet.h"

using namespace bs;

const Color GUIElement::kDisabledColor = Color(0.5f, 0.5f, 0.5f, 1.0f);

GUIElement::GUIElement(String styleName, const GUISizeConstraints& dimensions, GUIElementOptions options)
	: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(std::move(styleName))
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

GUIElement::GUIElement(const char* styleName, const GUISizeConstraints& dimensions, GUIElementOptions options)
	: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(styleName ? styleName : StringUtil::kBlank)
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

void GUIElement::UpdateRenderElements()
{
	UpdateClippedBounds();
}

void GUIElement::GetRenderElementVertexAndIndexData(u32 renderElementIndex, u32 vertexOffset, u32 indexOffset, DataRange& outPositions, DataRange& outUVs, DataRange& outIndices) const
{
	const GUIRenderElement& renderElement = mRenderElements[renderElementIndex];

	const Vector2I guiElementOffset(mLayoutData.Area.X, mLayoutData.Area.Y);
	const Rect2 guiElementClipRectangle = (Rect2)mLayoutData.GetLocalClipRect();

	// Build the render element bounds to use for clipping
	Rect2 renderElementBounds( // In space relative to parent GUI element
		renderElement.Offset.X,
		renderElement.Offset.Y,
		renderElement.ClipSize.Width,
		renderElement.ClipSize.Height);

	// Clip by the elements clip rectangle
	renderElementBounds.Clip(guiElementClipRectangle);

	// Move the bounds into space relative to the content bounds
	renderElementBounds.X -= renderElement.Offset.X;
	renderElementBounds.Y -= renderElement.Offset.Y;

	const Vector2 renderElementOffset =
		Vector2((float)guiElementOffset.X, (float)guiElementOffset.Y) +
		renderElement.Offset;

	renderElement.GetVertexAndIndexData(vertexOffset, indexOffset, renderElementOffset, renderElementBounds, true, outPositions, outUVs, outIndices);
}

void GUIElement::UpdateClippedBounds()
{
	mClippedBounds = mLayoutData.Area;
	mClippedBounds.Clip(mLayoutData.ClipRect);
}

void GUIElement::SetStyle(const String& styleName)
{
	mStyleName = styleName;
	NotifyStyleSheetChanged();
}

bool GUIElement::DoOnMouseEvent(const GUIMouseEvent& event)
{
	return false;
}

bool GUIElement::DoOnTextInputEvent(const GUITextInputEvent& event)
{
	return false;
}

bool GUIElement::DoOnCommandEvent(const GUICommandEvent& event)
{
	if(event.GetType() == GUICommandEventType::FocusGained)
	{
		OnFocusChanged(true);
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(event.GetType() == GUICommandEventType::FocusLost)
	{
		OnFocusChanged(false);
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

bool GUIElement::DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& event)
{
	return false;
}

void GUIElement::SetTint(const Color& color)
{
	mColor = color;

	MarkContentAsDirty();
}

void GUIElement::SetElementDepth(u8 depth)
{
	mLayoutData.Depth = depth | (mLayoutData.Depth & 0xFFFFFF00);
	MarkMeshAsDirty();
}

u8 GUIElement::GetElementDepth() const
{
	return mLayoutData.Depth & 0xFF;
}

void GUIElement::SetLayoutData(const GUILayoutData& data)
{
	// Preserve element depth as that is not controlled by layout but is stored
	// there only for convenience
	u8 elemDepth = GetElementDepth();
	GUIElementBase::SetLayoutData(data);
	SetElementDepth(elemDepth);

	UpdateClippedBounds();
}

void GUIElement::ChangeParentWidget(GUIWidget* widget)
{
	if(IsDestroyed())
		return;

	bool widgetChanged = false;
	if(mParentWidget != widget)
	{
		// Unregister from current widget's nav-group
		if(!mNavigationGroup && mParentWidget)
			mParentWidget->GetDefaultNavGroupInternal()->UnregisterElement(this);

		widgetChanged = true;
	}

	GUIElementBase::ChangeParentWidget(widget);

	if(widgetChanged)
	{
		// Register with the new widget's nav-group
		if(!mNavigationGroup && mParentWidget)
			mParentWidget->GetDefaultNavGroupInternal()->RegisterElement(this);

		NotifyStyleSheetChanged();
	}
}

const RectOffset& GUIElement::GetMargins() const
{
	if(mStyleSheetStateStyle)
		return mStyleSheetStateStyle->Margins;
	else if(mStyle != nullptr)
		return mStyle->Padding; // Note: Old GUI style has the meaning of padding/margins swapped
	else
	{
		static RectOffset margins;
		return margins;
	}
}

const RectOffset& GUIElement::GetPadding() const
{
	if(mStyleSheetStateStyle)
		return mStyleSheetStateStyle->Padding;
	else if(mStyle != nullptr)
		return mStyle->Margins; // Note: Old GUI style has the meaning of padding/margins swapped
	else
	{
		static RectOffset padding;
		return padding;
	}
}

void GUIElement::SetNavigationGroup(const SPtr<GUINavGroup>& navGroup)
{
	SPtr<GUINavGroup> currentNavGroup = GetNavigationGroup();
	if(currentNavGroup == navGroup)
		return;

	if(currentNavGroup)
		currentNavGroup->UnregisterElement(this);

	if(navGroup)
		navGroup->RegisterElement(this);

	mNavigationGroup = navGroup;
}

void GUIElement::SetNavigationGroupIndex(i32 index)
{
	SPtr<GUINavGroup> navGroup = GetNavigationGroup();
	if(navGroup != nullptr)
		navGroup->SetIndex(this, index);
}

SPtr<GUINavGroup> GUIElement::GetNavigationGroup() const
{
	if(mNavigationGroup)
		return mNavigationGroup;

	if(mParentWidget)
		return mParentWidget->GetDefaultNavGroupInternal();

	return nullptr;
}

void GUIElement::SetFocus(bool enabled, bool clear)
{
	GUIManager::Instance().SetFocus(this, enabled, clear);
}

void GUIElement::ResetDimensions()
{
	const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	mSizeConstraints = GUISizeConstraints::Create();
	mSizeConstraints.UpdateWithStyle(mStyle);

	const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

Rect2I GUIElement::GetCachedContentBounds() const
{
	if(mStyleSheetStateStyle != nullptr)
	{
		const RectOffset& padding = GetPadding();
		const u32 paddingWidth = padding.Left + padding.Right;
		const u32 paddingHeight = padding.Top + padding.Bottom;

		const u32 borderWidth = mStyleSheetStateStyle->BorderLeft.GetVisibleWidth() + mStyleSheetStateStyle->BorderRight.GetVisibleWidth();
		const u32 borderHeight = mStyleSheetStateStyle->BorderTop.GetVisibleWidth() + mStyleSheetStateStyle->BorderBottom.GetVisibleWidth();

		Rect2I bounds = GetCachedBounds();
		const u32 nonContentWidth = Math::Min(bounds.Width, paddingWidth + borderWidth);
		const u32 nonContentHeight = Math::Min(bounds.Height, paddingHeight + borderHeight);

		bounds.X += (i32)Math::Min(bounds.Width, padding.Left + mStyleSheetStateStyle->BorderLeft.GetVisibleWidth());
		bounds.Y += (i32)Math::Min(bounds.Height, padding.Top + mStyleSheetStateStyle->BorderTop.GetVisibleWidth());
		bounds.Width -= nonContentWidth;
		bounds.Height -= nonContentHeight;

		return bounds;
	}
	else
	{
		const RectOffset& padding = GetPadding();
		Rect2I bounds;

		bounds.X = mLayoutData.Area.X + padding.Left + mStyle->ContentOffset.Left;
		bounds.Y = mLayoutData.Area.Y + padding.Top + mStyle->ContentOffset.Top;
		bounds.Width = (u32)std::max(0, (i32)mLayoutData.Area.Width - (i32)(padding.Left + padding.Right + mStyle->ContentOffset.Left + mStyle->ContentOffset.Right));
		bounds.Height = (u32)std::max(0, (i32)mLayoutData.Area.Height - (i32)(padding.Top + padding.Bottom + mStyle->ContentOffset.Top + mStyle->ContentOffset.Bottom));

		return bounds;
	}
}

Rect2I GUIElement::GetCachedClippedContentBoundsInContentSpace() const
{
	const Vector2I contentOffset = GetContentOffsetInElementSpace();
	const Rect2I contentBounds = GetCachedContentBounds();

	// Transform into element space so we can clip it using the element clip rectangle
	Rect2I contentClipRect(contentOffset.X, contentOffset.Y, contentBounds.Width, contentBounds.Height);
	contentClipRect.Clip(mLayoutData.GetLocalClipRect());

	// Transform into content sprite space
	contentClipRect.X -= contentOffset.X;
	contentClipRect.Y -= contentOffset.Y;

	return contentClipRect;
}

Vector2I GUIElement::GetContentOffsetInElementSpace() const
{
	const RectOffset& padding = GetPadding();
	if(mStyleSheetStateStyle != nullptr)
	{
		return Vector2I(
			padding.Left + mStyleSheetStateStyle->BorderLeft.GetVisibleWidth(),
			padding.Top + mStyleSheetStateStyle->BorderTop.GetVisibleWidth());
	}
	else
	{
		return Vector2I(
			padding.Left + mStyle->ContentOffset.Left,
			padding.Top + mStyle->ContentOffset.Top);
	}
}

Color GUIElement::GetTint() const
{
	if(mStyleSheetStateStyle != nullptr) // With style sheets, disabled color is controlled via a separate rule, rather than being hardcoded
		return mColor;
	else
	{
		if(!IsDisabled())
			return mColor;

		return mColor * kDisabledColor;
	}
}

void GUIElement::AddStateFlags(GUIElementStateFlags flags)
{
	if(mStateFlags.IsSetAll(flags))
		return;

	mStateFlags |= flags;

	if(mStyleSheetStyle != nullptr)
		mStyleSheetStateStyle = mStyleSheetStyle->FindStateStyle(mStateFlags);
	else
		mStyleSheetStateStyle = GUIStyleSheetStateRule::kDefault;

	MarkContentAsDirty();
}

void GUIElement::RemoveStateFlags(GUIElementStateFlags flags)
{
	if(!mStateFlags.IsSetAny(flags))
		return;

	mStateFlags &= ~flags;

	if(mStyleSheetStyle != nullptr)
		mStyleSheetStateStyle = mStyleSheetStyle->FindStateStyle(mStateFlags);
	else
		mStyleSheetStateStyle = GUIStyleSheetStateRule::kDefault;

	MarkContentAsDirty();
}

bool GUIElement::IsInBounds(const Vector2I position) const
{
	return GetCachedClippedBounds().Contains(position);
}

SPtr<GUIContextMenu> GUIElement::GetContextMenu() const
{
	if(!IsDisabled())
		return mContextMenu;

	return nullptr;
}

void GUIElement::NotifyStyleSheetChanged()
{
	const char* styleSheetElement = GetStyleSheetElement();
	const bool isUsingStyleSheets = styleSheetElement != nullptr;

	if(isUsingStyleSheets)
	{
		HGUIStyleSheet styleSheet;
		if(GetParentWidget())
			styleSheet = GetParentWidget()->GetStyleSheet();

		SPtr<GUIStyleSheetRule> newStyle;
		if(styleSheet.IsLoaded(false))
			newStyle = styleSheet->BuildRule(*this);

		if(!newStyle)
			newStyle = GUIStyleSheetRule::kDefault;

		if(newStyle != mStyleSheetStyle)
		{
			mStyleSheetStyle = newStyle;

			const bool isFixedBefore = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();

			mStyleSheetStateStyle = mStyleSheetStyle->FindStateStyle(mStateFlags);
			mSizeConstraints.UpdateWithStyle(*mStyleSheetStateStyle);

			const bool isFixedAfter = mSizeConstraints.IsWidthFixed() && mSizeConstraints.IsHeightFixed();
			if(isFixedBefore != isFixedAfter)
				RefreshLayoutUpdateParentsForChildren();

			NotifyStyleChanged();
			MarkLayoutAsDirty();
		}
	}
	else
	{
		const GUIElementStyle* newStyle = nullptr;
		if(GetParentWidget() != nullptr && !mStyleName.empty())
			newStyle = GetParentWidget()->GetSkin().GetStyle(mStyleName);
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

const String& GUIElement::GetSubStyleName(const String& subStyleTypeName) const
{
	auto iterFind = mStyle->SubStyles.find(subStyleTypeName);

	if(iterFind != mStyle->SubStyles.end())
		return iterFind->second;
	else
		return StringUtil::kBlank;
}

void GUIElement::Destroy(GUIElement* element)
{
	if(element->mIsDestroyed)
		return;

	SPtr<GUINavGroup> currentNavGroup = element->GetNavigationGroup();
	if(currentNavGroup)
		currentNavGroup->UnregisterElement(element);

	if(element->mParent != nullptr)
		element->mParent->UnregisterChildElement(element);

	element->mIsDestroyed = true;

	GUIManager::Instance().QueueForDestroy(element);
}
