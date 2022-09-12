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
		:GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle), mStyleName(std::move(styleName))
	{
		// Style is set to default here, and the proper one is assigned once GUI element
		// is assigned to a parent (that's when the active GUI skin becomes known)
	}

	GUIElement::GUIElement(const char* styleName, const GUIDimensions& dimensions, GUIElementOptions options)
		: GUIElementBase(dimensions), mOptionFlags(options), mStyle(&GUISkin::DefaultStyle)
		, MStyleName(styleName ? styleName : StringUtil::BLANK)
	{
		// Style is set to default here, and the proper one is assigned once GUI element
		// is assigned to a parent (that's when the active GUI skin becomes known)
	}

	void GUIElement::_updateRenderElements()
	{
		updateRenderElementsInternal();
	}

	void GUIElement::UpdateRenderElementsInternal()
	{
		updateClippedBounds();
	}

	void GUIElement::UpdateClippedBounds()
	{
		mClippedBounds = mLayoutData.area;
		mClippedBounds.Clip(mLayoutData.clipRect);
	}

	void GUIElement::SetStyle(const String& styleName)
	{
		mStyleName = styleName;
		_refreshStyle();
	}

	bool GUIElement::_mouseEvent(const GUIMouseEvent& ev)
	{
		return false;
	}

	bool GUIElement::_textInputEvent(const GUITextInputEvent& ev)
	{
		return false;
	}

	bool GUIElement::_commandEvent(const GUICommandEvent& ev)
	{
		if (ev.GetType() == GUICommandEventType::FocusGained)
		{
			onFocusChanged(true);
			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}
		else if (ev.GetType() == GUICommandEventType::FocusLost)
		{
			onFocusChanged(false);
			return !mOptionFlags.IsSet(GUIElementOption::ClickThrough);
		}

		return false;
	}

	bool GUIElement::_virtualButtonEvent(const GUIVirtualButtonEvent& ev)
	{
		return false;
	}

	void GUIElement::SetTint(const Color& color)
	{
		mColor = color;

		_markContentAsDirty();
	}

	void GUIElement::_setElementDepth(UINT8 depth)
	{
		mLayoutData.depth = depth | (mLayoutData.depth & 0xFFFFFF00);
		_markMeshAsDirty();
	}

	UINT8 GUIElement::_getElementDepth() const
	{
		return mLayoutData.depth & 0xFF;
	}

	void GUIElement::_setLayoutData(const GUILayoutData& data)
	{
		// Preserve element depth as that is not controlled by layout but is stored
		// there only for convenience
		UINT8 elemDepth = _getElementDepth();
		GUIElementBase::_setLayoutData(data);
		_setElementDepth(elemDepth);

		updateClippedBounds();
	}

	void GUIElement::_changeParentWidget(GUIWidget* widget)
	{
		if (_isDestroyed())
			return;

		bool widgetChanged = false;
		if(mParentWidget != widget)
		{
			// Unregister from current widget's nav-group
			if(!mNavGroup && mParentWidget)
				mParentWidget->_getDefaultNavGroup()->UnregisterElement(this);

			widgetChanged = true;
		}

		GUIElementBase::_changeParentWidget(widget);

		if(widgetChanged)
		{
			// Register with the new widget's nav-group
			if(!mNavGroup && mParentWidget)
				mParentWidget->_getDefaultNavGroup()->RegisterElement(this);

			_refreshStyle();
		}
	}

	const RectOffset& GUIElement::_getPadding() const
	{
		if(mStyle != nullptr)
			return mStyle->padding;
		else
		{
			static RectOffset padding;

			return padding;
		}
	}

	void GUIElement::SetNavGroup(const SPtr<GUINavGroup>& navGroup)
	{
		SPtr<GUINavGroup> currentNavGroup = _getNavGroup();
		if(currentNavGroup == navGroup)
			return;

		if(currentNavGroup)
			currentNavGroup->UnregisterElement(this);

		if(navGroup)
			navGroup->RegisterElement(this);

		mNavGroup = navGroup;
	}

	void GUIElement::SetNavGroupIndex(INT32 index)
	{
		SPtr<GUINavGroup> navGroup = _getNavGroup();
		if(navGroup != nullptr)
			navGroup->SetIndex(this, index);
	}

	SPtr<GUINavGroup> GUIElement::_getNavGroup() const
	{
		if(mNavGroup)
			return mNavGroup;

		if(mParentWidget)
			return mParentWidget->_getDefaultNavGroup();

		return nullptr;
	}

	void GUIElement::SetFocus(bool enabled, bool clear)
	{
		GUIManager::instance().SetFocus(this, enabled, clear);
	}

	void GUIElement::ResetDimensions()
	{
		bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		mDimensions = GUIDimensions::create();
		mDimensions.UpdateWithStyle(mStyle);

		bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

		if (isFixedBefore != isFixedAfter)
			refreshChildUpdateParents();

		_markLayoutAsDirty();
	}

	Rect2I GUIElement::GetCachedVisibleBounds() const
	{
		Rect2I bounds = _getClippedBounds();
		
		bounds.x += mStyle->margins.left;
		bounds.y += mStyle->margins.top;
		bounds.width = (UINT32)std::max(0, (INT32)bounds.width - (INT32)(mStyle->margins.left + mStyle->margins.right));
		bounds.height = (UINT32)std::max(0, (INT32)bounds.height - (INT32)(mStyle->margins.top + mStyle->margins.bottom));

		return bounds;
	}

	Rect2I GUIElement::GetCachedContentBounds() const
	{
		Rect2I bounds;

		bounds.x = mLayoutData.area.x + mStyle->margins.left + mStyle->contentOffset.left;
		bounds.y = mLayoutData.area.y + mStyle->margins.top + mStyle->contentOffset.top;
		bounds.width = (UINT32)std::max(0, (INT32)mLayoutData.area.width -
			(INT32)(mStyle->margins.left + mStyle->margins.right + mStyle->contentOffset.left + mStyle->contentOffset.right));
		bounds.height = (UINT32)std::max(0, (INT32)mLayoutData.area.height -
			(INT32)(mStyle->margins.top + mStyle->margins.bottom + mStyle->contentOffset.top + mStyle->contentOffset.bottom));

		return bounds;
	}

	Rect2I GUIElement::GetCachedContentClipRect() const
	{
		Rect2I contentBounds = getCachedContentBounds();
		
		// Transform into element space so we can clip it using the element clip rectangle
		Vector2I offsetDiff = Vector2I(contentBounds.x - mLayoutData.area.x, contentBounds.y - mLayoutData.area.y);
		Rect2I ContentClipRect(offsetDiff.x, offsetDiff.y, contentBounds.width, contentBounds.height);
		contentClipRect.Clip(mLayoutData.getLocalClipRect());

		// Transform into content sprite space
		contentClipRect.x -= offsetDiff.x;
		contentClipRect.y -= offsetDiff.y;

		return contentClipRect;
	}

	Color GUIElement::GetTint() const
	{
		if (!_isDisabled())
			return mColor;

		return mColor * DISABLED_COLOR;
	}

	bool GUIElement::_isInBounds(const Vector2I position) const
	{
		Rect2I contentBounds = getCachedVisibleBounds();

		return contentBounds.Contains(position);
	}

	SPtr<GUIContextMenu> GUIElement::_getContextMenu() const
	{
		if (!_isDisabled())
			return mContextMenu;

		return nullptr;
	}

	void GUIElement::_refreshStyle()
	{
		const GUIElementStyle* newStyle = nullptr;
		if(_getParentWidget() != nullptr && !mStyleName.empty())
			newStyle = _getParentWidget()->GetSkin().GetStyle(mStyleName);
		else
			newStyle = &GUISkin::DefaultStyle;

		if(newStyle != mStyle)
		{
			mStyle = newStyle;

			bool isFixedBefore = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;

			mDimensions.UpdateWithStyle(mStyle);

			bool isFixedAfter = (mDimensions.flags & GUIDF_FixedWidth) != 0 && (mDimensions.flags & GUIDF_FixedHeight) != 0;
			if (isFixedBefore != isFixedAfter)
				refreshChildUpdateParents();

			styleUpdated();
			_markLayoutAsDirty();
		}
	}

	const String& GUIElement::GetSubStyleName(const String& subStyleTypeName) const
	{
		auto iterFind = mStyle->subStyles.find(subStyleTypeName);

		if (iterFind != mStyle->subStyles.end())
			return iterFind->second;
		else
			return StringUtil::BLANK;
	}

	void GUIElement::Destroy(GUIElement* element)
	{
		if(element->mIsDestroyed)
			return;

		SPtr<GUINavGroup> currentNavGroup = element->_getNavGroup();
		if(currentNavGroup)
			currentNavGroup->UnregisterElement(element);

		if (element->mParentElement != nullptr)
			element->mParentElement->_unregisterChildElement(element);

		element->mIsDestroyed = true;

		GUIManager::instance().QueueForDestroy(element);
	}

	Rect2I GUIElement::GetVisibleBounds()
	{
		Rect2I bounds = getBounds();

		bounds.x += mStyle->margins.left;
		bounds.y += mStyle->margins.top;
		bounds.width = (UINT32)std::max(0, (INT32)bounds.width - (INT32)(mStyle->margins.left + mStyle->margins.right));
		bounds.height = (UINT32)std::max(0, (INT32)bounds.height - (INT32)(mStyle->margins.top + mStyle->margins.bottom));

		return bounds;
	}
}
