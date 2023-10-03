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

GUIElement::GUIElement(String styleName, const GUIDimensions& dimensions, GUIElementOptions options)
	: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(std::move(styleName))
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

GUIElement::GUIElement(const char* styleName, const GUIDimensions& dimensions, GUIElementOptions options)
	: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(styleName ? styleName : StringUtil::kBlank)
{
	// Style is set to default here, and the proper one is assigned once GUI element
	// is assigned to a parent (that's when the active GUI skin becomes known)
}

void GUIElement::UpdateRenderElements()
{
	UpdateClippedBounds();
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
	bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	mDimensions = GUIDimensions::Create();
	mDimensions.UpdateWithStyle(mStyle);

	bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

	if(isFixedBefore != isFixedAfter)
		RefreshLayoutUpdateParentsForChildren();

	MarkLayoutAsDirty();
}

Rect2I GUIElement::GetCachedVisibleBounds() const
{
	const RectOffset& margins = GetPadding();
	Rect2I bounds = GetClippedBounds();

	bounds.X += margins.Left;
	bounds.Y += margins.Top;
	bounds.Width = (u32)std::max(0, (i32)bounds.Width - (i32)(margins.Left + margins.Right));
	bounds.Height = (u32)std::max(0, (i32)bounds.Height - (i32)(margins.Top + margins.Bottom));

	return bounds;
}

Rect2I GUIElement::GetCachedContentBounds() const
{
	const RectOffset& margins = GetPadding();
	Rect2I bounds;

	bounds.X = mLayoutData.Area.X + margins.Left + mStyle->ContentOffset.Left;
	bounds.Y = mLayoutData.Area.Y + margins.Top + mStyle->ContentOffset.Top;
	bounds.Width = (u32)std::max(0, (i32)mLayoutData.Area.Width - (i32)(margins.Left + margins.Right + mStyle->ContentOffset.Left + mStyle->ContentOffset.Right));
	bounds.Height = (u32)std::max(0, (i32)mLayoutData.Area.Height - (i32)(margins.Top + margins.Bottom + mStyle->ContentOffset.Top + mStyle->ContentOffset.Bottom));

	return bounds;
}

Rect2I GUIElement::GetCachedContentClipRect() const
{
	Rect2I contentBounds = GetCachedContentBounds();

	// Transform into element space so we can clip it using the element clip rectangle
	Vector2I offsetDiff = Vector2I(contentBounds.X - mLayoutData.Area.X, contentBounds.Y - mLayoutData.Area.Y);
	Rect2I contentClipRect(offsetDiff.X, offsetDiff.Y, contentBounds.Width, contentBounds.Height);
	contentClipRect.Clip(mLayoutData.GetLocalClipRect());

	// Transform into content sprite space
	contentClipRect.X -= offsetDiff.X;
	contentClipRect.Y -= offsetDiff.Y;

	return contentClipRect;
}

Color GUIElement::GetTint() const
{
	if(!IsDisabled())
		return mColor;

	return mColor * kDisabledColor;
}

void GUIElement::AddStateFlags(GUIElementStateFlags flags)
{
	if(mStateFlags.IsSetAll(flags))
		return;

	mStateFlags |= flags;

	if(mStyleSheetStyle != nullptr)
		mStyleSheetStateStyle = mStyleSheetStyle->FindStateStyle(mStateFlags);
	else
		mStyleSheetStateStyle = GUIStyleSheetStateStyle::kDefault;

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
		mStyleSheetStateStyle = GUIStyleSheetStateStyle::kDefault;

	MarkContentAsDirty();
}

bool GUIElement::IsInBounds(const Vector2I position) const
{
	Rect2I contentBounds = GetCachedVisibleBounds();

	return contentBounds.Contains(position);
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

		SPtr<GUIStyleSheetStyle> newStyle;
		if(styleSheet.IsLoaded(false))
			newStyle = styleSheet->FindStyle(styleSheetElement, GetStyleSheetId());

		if(!newStyle)
			newStyle = GUIStyleSheetStyle::kDefault;

		if(newStyle != mStyleSheetStyle)
		{
			mStyleSheetStyle = newStyle;

			bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

			mStyleSheetStateStyle = mStyleSheetStyle->FindStateStyle(mStateFlags);
			mDimensions.UpdateWithStyle(*mStyleSheetStateStyle);

			bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;
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

			bool isFixedBefore = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;

			mDimensions.UpdateWithStyle(mStyle);

			bool isFixedAfter = (mDimensions.Flags & GUIDF_FixedWidth) != 0 && (mDimensions.Flags & GUIDF_FixedHeight) != 0;
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

Rect2I GUIElement::GetVisibleBounds()
{
	const RectOffset& margins = GetPadding();
	Rect2I bounds = GetBounds();

	bounds.X += margins.Left;
	bounds.Y += margins.Top;
	bounds.Width = (u32)std::max(0, (i32)bounds.Width - (i32)(margins.Left + margins.Right));
	bounds.Height = (u32)std::max(0, (i32)bounds.Height - (i32)(margins.Top + margins.Bottom));

	return bounds;
}
