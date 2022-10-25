//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIElement.h"
#include "GUI/BsGUIWidget.h"
#include "GUI/BsGUISkin.h"
#include "GUI/BsGUIManager.h"
#include "BsGUINavGroup.h"

namespace bs
{
	const Color GUIElement::DISABLED_COLOR = Color(0.5f, 0.5f, 0.5f, 1.0f);

	GUIElement::GUIElement(String styleName, const GUIDimensions& dimensions, GUIElementOptions options)
		: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(std::move(styleName))
	{
		// Style is set to default here, and the proper one is assigned once GUI element
		// is assigned to a parent (that's when the active GUI skin becomes known)
	}

	GUIElement::GUIElement(const char* styleName, const GUIDimensions& dimensions, GUIElementOptions options)
		: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(styleName ? styleName : StringUtil::BLANK)
	{
		// Style is set to default here, and the proper one is assigned once GUI element
		// is assigned to a parent (that's when the active GUI skin becomes known)
	}

	void GUIElement::UpdateRenderElementsInternal()
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
		RefreshStyleInternal();
	}

	bool GUIElement::MouseEventInternal(const GUIMouseEvent& ev)
	{
		return false;
	}

	bool GUIElement::TextInputEventInternal(const GUITextInputEvent& ev)
	{
		return false;
	}

	bool GUIElement::CommandEventInternal(const GUICommandEvent& ev)
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

	bool GUIElement::VirtualButtonEventInternal(const GUIVirtualButtonEvent& ev)
	{
		return false;
	}

	void GUIElement::SetTint(const Color& color)
	{
		mColor = color;

		MarkContentAsDirtyInternal();
	}

	void GUIElement::SetElementDepthInternal(u8 depth)
	{
		mLayoutData.Depth = depth | (mLayoutData.Depth & 0xFFFFFF00);
		MarkMeshAsDirtyInternal();
	}

	u8 GUIElement::GetElementDepthInternal() const
	{
		return mLayoutData.Depth & 0xFF;
	}

	void GUIElement::SetLayoutDataInternal(const GUILayoutData& data)
	{
		// Preserve element depth as that is not controlled by layout but is stored
		// there only for convenience
		u8 elemDepth = GetElementDepthInternal();
		GUIElementBase::SetLayoutDataInternal(data);
		SetElementDepthInternal(elemDepth);

		UpdateClippedBounds();
	}

	void GUIElement::ChangeParentWidgetInternal(GUIWidget* widget)
	{
		if(IsDestroyedInternal())
			return;

		bool widgetChanged = false;
		if(mParentWidget != widget)
		{
			// Unregister from current widget's nav-group
			if(!mNavGroup && mParentWidget)
				mParentWidget->GetDefaultNavGroupInternal()->UnregisterElement(this);

			widgetChanged = true;
		}

		GUIElementBase::ChangeParentWidgetInternal(widget);

		if(widgetChanged)
		{
			// Register with the new widget's nav-group
			if(!mNavGroup && mParentWidget)
				mParentWidget->GetDefaultNavGroupInternal()->RegisterElement(this);

			RefreshStyleInternal();
		}
	}

	const RectOffset& GUIElement::GetPaddingInternal() const
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
		SPtr<GUINavGroup> currentNavGroup = GetNavGroupInternal();
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
		SPtr<GUINavGroup> navGroup = GetNavGroupInternal();
		if(navGroup != nullptr)
			navGroup->SetIndex(this, index);
	}

	SPtr<GUINavGroup> GUIElement::GetNavGroupInternal() const
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

		MarkLayoutAsDirtyInternal();
	}

	Rect2I GUIElement::GetCachedVisibleBounds() const
	{
		Rect2I bounds = GetClippedBoundsInternal();

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
		if(!IsDisabledInternal())
			return mColor;

		return mColor * DISABLED_COLOR;
	}

	bool GUIElement::IsInBoundsInternal(const Vector2I position) const
	{
		Rect2I contentBounds = GetCachedVisibleBounds();

		return contentBounds.Contains(position);
	}

	SPtr<GUIContextMenu> GUIElement::GetContextMenuInternal() const
	{
		if(!IsDisabledInternal())
			return mContextMenu;

		return nullptr;
	}

	void GUIElement::RefreshStyleInternal()
	{
		const GUIElementStyle* newStyle = nullptr;
		if(GetParentWidgetInternal() != nullptr && !mStyleName.empty())
			newStyle = GetParentWidgetInternal()->GetSkin().GetStyle(mStyleName);
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
			MarkLayoutAsDirtyInternal();
		}
	}

	const String& GUIElement::GetSubStyleName(const String& subStyleTypeName) const
	{
		auto iterFind = mStyle->SubStyles.find(subStyleTypeName);

		if(iterFind != mStyle->SubStyles.end())
			return iterFind->second;
		else
			return StringUtil::BLANK;
	}

	void GUIElement::Destroy(GUIElement* element)
	{
		if(element->mIsDestroyed)
			return;

		SPtr<GUINavGroup> currentNavGroup = element->GetNavGroupInternal();
		if(currentNavGroup)
			currentNavGroup->UnregisterElement(element);

		if(element->mParentElement != nullptr)
			element->mParentElement->UnregisterChildElementInternal(element);

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
} // namespace bs
