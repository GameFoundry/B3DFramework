//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIManager.h"
#include "BsGUINavGroup.h"

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
	RefreshStyle();
}

bool GUIElement::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	return false;
}

bool GUIElement::DoOnTextInputEvent(const GUITextInputEvent& ev)
{
	return false;
}

bool GUIElement::DoOnCommandEvent(const GUICommandEvent& ev)
{
	if(ev.GetType() == GUICommandEventType::FocusGained)
	{
		OnFocusChanged(true);
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}
	else if(ev.GetType() == GUICommandEventType::FocusLost)
	{
		OnFocusChanged(false);
		return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
	}

	return false;
}

bool GUIElement::DoOnVirtualButtonEvent(const GUIVirtualButtonEvent& ev)
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
		if(!mNavGroup && mParentWidget)
			mParentWidget->GetDefaultNavGroupInternal()->UnregisterElement(this);

		widgetChanged = true;
	}

	GUIElementBase::ChangeParentWidget(widget);

	if(widgetChanged)
	{
		// Register with the new widget's nav-group
		if(!mNavGroup && mParentWidget)
			mParentWidget->GetDefaultNavGroupInternal()->RegisterElement(this);

		RefreshStyle();
	}
}

const RectOffset& GUIElement::GetPadding() const
{
	if(mStyle != nullptr)
		return mStyle->Padding;
	else
	{
		static RectOffset padding;

		return padding;
	}
}

void GUIElement::SetNavGroup(const SPtr<GUINavGroup>& navGroup)
{
	SPtr<GUINavGroup> currentNavGroup = GetNavGroup();
	if(currentNavGroup == navGroup)
		return;

	if(currentNavGroup)
		currentNavGroup->UnregisterElement(this);

	if(navGroup)
		navGroup->RegisterElement(this);

	mNavGroup = navGroup;
}

void GUIElement::SetNavGroupIndex(i32 index)
{
	SPtr<GUINavGroup> navGroup = GetNavGroup();
	if(navGroup != nullptr)
		navGroup->SetIndex(this, index);
}

SPtr<GUINavGroup> GUIElement::GetNavGroup() const
{
	if(mNavGroup)
		return mNavGroup;

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
		RefreshChildUpdateParents();

	MarkLayoutAsDirty();
}

Rect2I GUIElement::GetCachedVisibleBounds() const
{
	Rect2I bounds = GetClippedBounds();

	bounds.X += mStyle->Margins.Left;
	bounds.Y += mStyle->Margins.Top;
	bounds.Width = (u32)std::max(0, (i32)bounds.Width - (i32)(mStyle->Margins.Left + mStyle->Margins.Right));
	bounds.Height = (u32)std::max(0, (i32)bounds.Height - (i32)(mStyle->Margins.Top + mStyle->Margins.Bottom));

	return bounds;
}

Rect2I GUIElement::GetCachedContentBounds() const
{
	Rect2I bounds;

	bounds.X = mLayoutData.Area.X + mStyle->Margins.Left + mStyle->ContentOffset.Left;
	bounds.Y = mLayoutData.Area.Y + mStyle->Margins.Top + mStyle->ContentOffset.Top;
	bounds.Width = (u32)std::max(0, (i32)mLayoutData.Area.Width - (i32)(mStyle->Margins.Left + mStyle->Margins.Right + mStyle->ContentOffset.Left + mStyle->ContentOffset.Right));
	bounds.Height = (u32)std::max(0, (i32)mLayoutData.Area.Height - (i32)(mStyle->Margins.Top + mStyle->Margins.Bottom + mStyle->ContentOffset.Top + mStyle->ContentOffset.Bottom));

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

void GUIElement::RefreshStyle()
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
			RefreshChildUpdateParents();

		StyleUpdated();
		MarkLayoutAsDirty();
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

	SPtr<GUINavGroup> currentNavGroup = element->GetNavGroup();
	if(currentNavGroup)
		currentNavGroup->UnregisterElement(element);

	if(element->mParentElement != nullptr)
		element->mParentElement->UnregisterChildElement(element);

	element->mIsDestroyed = true;

	GUIManager::Instance().QueueForDestroy(element);
}

Rect2I GUIElement::GetVisibleBounds()
{
	Rect2I bounds = GetBounds();

	bounds.X += mStyle->Margins.Left;
	bounds.Y += mStyle->Margins.Top;
	bounds.Width = (u32)std::max(0, (i32)bounds.Width - (i32)(mStyle->Margins.Left + mStyle->Margins.Right));
	bounds.Height = (u32)std::max(0, (i32)bounds.Height - (i32)(mStyle->Margins.Top + mStyle->Margins.Bottom));

	return bounds;
}
