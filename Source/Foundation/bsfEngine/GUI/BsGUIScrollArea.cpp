//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollArea.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUIDimensions.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIScrollBarVert.h"
#include "GUI/BsGUIScrollBarHorz.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUILayoutUtility.h"

using namespace std::placeholders;

namespace bs
{
	const UINT32 GUIScrollArea::ScrollBarWidth = 16;
	const UINT32 GUIScrollArea::WheelScrollAmount = 50;

	GUIScrollArea::GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType,
		const String& scrollBarStyle, const String& scrollAreaStyle, const GUIDimensions& dimensions)
		: GUIElementContainer(dimensions), mVertBarType(vertBarType), mHorzBarType(horzBarType)
		, MScrollBarStyle(scrollBarStyle), mVertScroll(nullptr), mHorzScroll(nullptr), mVertOffset(0), mHorzOffset(0)
		, MRecalculateVertOffset(false), mRecalculateHorzOffset(false)
	{
		mContentLayout = GUILayoutY::create();
		_registerChildElement(mContentLayout);

		mHorzScroll = GUIScrollBarHorz::create(mScrollBarStyle);
		mVertScroll = GUIScrollBarVert::create(mScrollBarStyle);

		_registerChildElement(mHorzScroll);
		_registerChildElement(mVertScroll);

		mHorzScroll->onScrollOrResize.Connect(std::bind(&GUIScrollArea::horzScrollUpdate, this, _1));
		mVertScroll->onScrollOrResize.Connect(std::bind(&GUIScrollArea::vertScrollUpdate, this, _1));
	}

	void GUIScrollArea::UpdateClippedBounds()
	{
		mClippedBounds = mLayoutData.area;
		mClippedBounds.Clip(mLayoutData.clipRect);
	}

	Vector2I GUIScrollArea::_getOptimalSize() const
	{
		Vector2I optimalSize = mContentLayout->_getOptimalSize();

		// Provide 10x10 in case underlying layout is empty because
		// 0 doesn't work well with the layout system
		optimalSize.x = std::max(10, optimalSize.x);
		optimalSize.y = std::max(10, optimalSize.y);

		return optimalSize;
	}

	LayoutSizeRange GUIScrollArea::_calculateLayoutSizeRange() const
	{
		// I'm ignoring scroll bars here since if the content layout fits
		// then they're not needed and the range is valid. And if it doesn't
		// fit the area will get clipped anyway and including the scroll bars
		// won't change the size much, but it would complicate this method significantly.
		if (mContentLayout->_isActive())
			return mDimensions.CalculateSizeRange(_getOptimalSize());

		return mDimensions.CalculateSizeRange(Vector2I());
	}

	LayoutSizeRange GUIScrollArea::_getLayoutSizeRange() const
	{
		return mSizeRange;
	}

	void GUIScrollArea::_updateOptimalLayoutSizes()
	{
		// Update all children first, otherwise we can't determine our own optimal size
		GUIElementBase::_updateOptimalLayoutSizes();

		if (mChildren.size() != mChildSizeRanges.size())
			mChildSizeRanges.Resize(mChildren.size());

		UINT32 childIdx = 0;
		for (auto& child : mChildren)
		{
			if (child->_isActive())
				mChildSizeRanges[childIdx] = child->_getLayoutSizeRange();
			else
				mChildSizeRanges[childIdx] = LayoutSizeRange();

			childIdx++;
		}

		mSizeRange = mDimensions.CalculateSizeRange(_getOptimalSize());
	}

	void GUIScrollArea::_getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const
	{
		Vector2I visibleSize, contentSize;
		_getElementAreas(layoutArea, elementAreas, numElements, sizeRanges, visibleSize, contentSize);
	}

	void GUIScrollArea::_getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
		const Vector<LayoutSizeRange>& sizeRanges, Vector2I& visibleSize, Vector2I& contentSize) const
	{
		assert(mChildren.size() == numElements && numElements == 3);

		UINT32 layoutIdx = 0;
		UINT32 horzScrollIdx = 0;
		UINT32 vertScrollIdx = 0;
		UINT32 idx = 0;
		for (auto& child : mChildren)
		{
			if (child == mContentLayout)
				layoutIdx = idx;

			if (child == mHorzScroll)
				horzScrollIdx = idx;

			if (child == mVertScroll)
				vertScrollIdx = idx;

			idx++;
		}

		// Calculate content layout bounds

		//// We want elements to use their optimal height, since scroll area
		//// technically provides "infinite" space
		UINT32 optimalContentWidth = layoutArea.width;
		if (mHorzBarType != ScrollBarType::NeverShow)
			optimalContentWidth = sizeRanges[layoutIdx].optimal.x;

		UINT32 optimalContentHeight = layoutArea.height;
		if (mVertBarType != ScrollBarType::NeverShow)
			optimalContentHeight = sizeRanges[layoutIdx].optimal.y;

		UINT32 layoutWidth = std::max(optimalContentWidth, (UINT32)layoutArea.width);
		UINT32 layoutHeight = std::max(optimalContentHeight, (UINT32)layoutArea.height);

		contentSize = GUILayoutUtility::calcActualSize(layoutWidth, layoutHeight, mContentLayout, false);
		visibleSize = Vector2I(layoutArea.width, layoutArea.height);

		bool addHorzScrollbar = (mHorzBarType == ScrollBarType::ShowIfDoesntFit && contentSize.x > visibleSize.x) ||
			mHorzBarType == ScrollBarType::AlwaysShow;

		bool hasHorzScrollbar = false;
		bool hasVertScrollbar = false;
		if (addHorzScrollbar)
		{
			// Make room for scrollbar
			visibleSize.y = (UINT32)std::max(0, (INT32)layoutArea.height - (INT32)ScrollBarWidth);
			optimalContentHeight = (UINT32)std::max(0, (INT32)optimalContentHeight - (INT32)ScrollBarWidth);

			if (sizeRanges[layoutIdx].min.y > 0)
				optimalContentHeight = std::max((UINT32)sizeRanges[layoutIdx].min.y, optimalContentHeight);

			layoutHeight = std::max(optimalContentHeight, (UINT32)visibleSize.y); // Never go below optimal size

			contentSize = GUILayoutUtility::calcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
			hasHorzScrollbar = true;
		}

		bool addVertScrollbar = (mVertBarType == ScrollBarType::ShowIfDoesntFit && contentSize.y > visibleSize.y) ||
			mVertBarType == ScrollBarType::AlwaysShow;

		if (addVertScrollbar)
		{
			// Make room for scrollbar
			visibleSize.x = (UINT32)std::max(0, (INT32)layoutArea.width - (INT32)ScrollBarWidth);
			optimalContentWidth = (UINT32)std::max(0, (INT32)optimalContentWidth - (INT32)ScrollBarWidth);

			if (sizeRanges[layoutIdx].min.x > 0)
				optimalContentWidth = std::max((UINT32)sizeRanges[layoutIdx].min.x, optimalContentWidth);

			layoutWidth = std::max(optimalContentWidth, (UINT32)visibleSize.x); // Never go below optimal size

			contentSize = GUILayoutUtility::calcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
			hasVertScrollbar = true;

			if (!hasHorzScrollbar) // Since width has been reduced, we need to check if we require the horizontal scrollbar
			{
				addHorzScrollbar = (mHorzBarType == ScrollBarType::ShowIfDoesntFit && contentSize.x > visibleSize.x) && mHorzBarType != ScrollBarType::NeverShow;

				if (addHorzScrollbar)
				{
					// Make room for scrollbar
					visibleSize.y = (UINT32)std::max(0, (INT32)layoutArea.height - (INT32)ScrollBarWidth);
					optimalContentHeight = (UINT32)std::max(0, (INT32)optimalContentHeight - (INT32)ScrollBarWidth);

					if (sizeRanges[layoutIdx].min.y > 0)
						optimalContentHeight = std::max((UINT32)sizeRanges[layoutIdx].min.y, optimalContentHeight);

					layoutHeight = std::max(optimalContentHeight, (UINT32)visibleSize.y); // Never go below optimal size

					contentSize = GUILayoutUtility::calcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
					hasHorzScrollbar = true;
				}
			}
		}

		elementAreas[layoutIdx] = Rect2I(layoutArea.x, layoutArea.y, layoutWidth, layoutHeight);

		// Calculate vertical scrollbar bounds
		if (hasVertScrollbar)
		{
			INT32 scrollBarOffset = (UINT32)std::max(0, (INT32)layoutArea.width - (INT32)ScrollBarWidth);
			UINT32 scrollBarHeight = layoutArea.height;
			if (hasHorzScrollbar)
				scrollBarHeight = (UINT32)std::max(0, (INT32)scrollBarHeight - (INT32)ScrollBarWidth);

			elementAreas[vertScrollIdx] = Rect2I(layoutArea.x + scrollBarOffset, layoutArea.y, ScrollBarWidth, scrollBarHeight);
		}
		else
		{
			elementAreas[vertScrollIdx] = Rect2I(layoutArea.x + layoutWidth, layoutArea.y, 0, 0);
		}

		// Calculate horizontal scrollbar bounds
		if (hasHorzScrollbar)
		{
			INT32 scrollBarOffset = (UINT32)std::max(0, (INT32)layoutArea.height - (INT32)ScrollBarWidth);
			UINT32 scrollBarWidth = layoutArea.width;
			if (hasVertScrollbar)
				scrollBarWidth = (UINT32)std::max(0, (INT32)scrollBarWidth - (INT32)ScrollBarWidth);

			elementAreas[horzScrollIdx] = Rect2I(layoutArea.x, layoutArea.y + scrollBarOffset, scrollBarWidth, ScrollBarWidth);
		}
		else
		{
			elementAreas[horzScrollIdx] = Rect2I(layoutArea.x, layoutArea.y + layoutHeight, 0, 0);
		}		
	}

	void GUIScrollArea::_updateLayoutInternal(const GUILayoutData& data)
	{
		UINT32 numElements = (UINT32)mChildren.size();
		Rect2I* elementAreas = nullptr;

		if (numElements > 0)
			elementAreas = bs_stack_new<Rect2I>(numElements);

		UINT32 layoutIdx = 0;
		UINT32 horzScrollIdx = 0;
		UINT32 vertScrollIdx = 0;
		for (UINT32 i = 0; i < numElements; i++)
		{
			GUIElementBase* child = _getChild(i);

			if (child == mContentLayout)
				layoutIdx = i;

			if (child == mHorzScroll)
				horzScrollIdx = i;

			if (child == mVertScroll)
				vertScrollIdx = i;
		}

		_getElementAreas(data.area, elementAreas, numElements, mChildSizeRanges, mVisibleSize, mContentSize);

		Rect2I& layoutBounds = elementAreas[layoutIdx];
		Rect2I& horzScrollBounds = elementAreas[horzScrollIdx];
		Rect2I& vertScrollBounds = elementAreas[vertScrollIdx];

		// Recalculate offsets in case scroll percent got updated externally (this needs to be delayed to this point because
		// at the time of the update content and visible sizes might be out of date).
		UINT32 scrollableHeight = (UINT32)std::max(0, INT32(mContentSize.y) - INT32(mVisibleSize.y));
		if (mRecalculateVertOffset)
		{
			mVertOffset = scrollableHeight * Math::Clamp01(mVertScroll->GetScrollPos());

			mRecalculateVertOffset = false;
		}

		UINT32 scrollableWidth = (UINT32)std::max(0, INT32(mContentSize.x) - INT32(mVisibleSize.x));
		if (mRecalculateHorzOffset)
		{
			mHorzOffset = scrollableWidth * Math::Clamp01(mHorzScroll->GetScrollPos());

			mRecalculateHorzOffset = false;
		}

		// Reset offset in case layout size changed so everything fits
		mVertOffset = Math::Clamp(mVertOffset, 0.0f, (float)scrollableHeight);
		mHorzOffset = Math::Clamp(mHorzOffset, 0.0f, (float)scrollableWidth);

		// Layout
		if (mContentLayout->_isActive())
		{
			layoutBounds.x -= Math::FloorToInt(mHorzOffset);
			layoutBounds.y -= Math::FloorToInt(mVertOffset);

			Rect2I layoutClipRect = data.clipRect;
			layoutClipRect.width = (UINT32)mVisibleSize.x;
			layoutClipRect.height = (UINT32)mVisibleSize.y;
			layoutClipRect.Clip(data.clipRect);

			GUILayoutData layoutData = data;
			layoutData.area = layoutBounds;
			layoutData.clipRect = layoutClipRect;

			mContentLayout->_setLayoutData(layoutData);
			mContentLayout->_updateLayoutInternal(layoutData);
		}

		// Vertical scrollbar
		{
			GUILayoutData vertScrollData = data;
			vertScrollData.area = vertScrollBounds;

			vertScrollData.clipRect = vertScrollBounds;
			vertScrollData.clipRect.Clip(data.clipRect);

			mVertScroll->_setLayoutData(vertScrollData);
			mVertScroll->_updateLayoutInternal(vertScrollData);

			// Set new handle size and update position to match the new size
			UINT32 scrollableHeight = (UINT32)std::max(0, INT32(mContentSize.y) - INT32(vertScrollBounds.height));
			float newScrollPct = 0.0f;

			if (scrollableHeight > 0)
				newScrollPct = mVertOffset / scrollableHeight;	

			mVertScroll->_setHandleSize(vertScrollBounds.height / (float)mContentSize.y);
			mVertScroll->_setScrollPos(newScrollPct);
		}

		// Horizontal scrollbar
		{
			GUILayoutData horzScrollData = data;
			horzScrollData.area = horzScrollBounds;

			horzScrollData.clipRect = horzScrollBounds;
			horzScrollData.clipRect.Clip(data.clipRect);

			mHorzScroll->_setLayoutData(horzScrollData);
			mHorzScroll->_updateLayoutInternal(horzScrollData);

			// Set new handle size and update position to match the new size
			UINT32 scrollableWidth = (UINT32)std::max(0, INT32(mContentSize.x) - INT32(horzScrollBounds.width));
			float newScrollPct = 0.0f;

			if (scrollableWidth > 0)
				newScrollPct = mHorzOffset / scrollableWidth;

			mHorzScroll->_setHandleSize(horzScrollBounds.width / (float)mContentSize.x);
			mHorzScroll->_setScrollPos(newScrollPct);
		}

		if (elementAreas != nullptr)
			bs_stack_free(elementAreas);
	}

	void GUIScrollArea::VertScrollUpdate(float scrollPos)
	{
		UINT32 scrollableHeight = (UINT32)std::max(0, INT32(mContentSize.y) - INT32(mVisibleSize.y));
		mVertOffset = scrollableHeight * Math::Clamp01(scrollPos);

		_markLayoutAsDirty();
	}

	void GUIScrollArea::HorzScrollUpdate(float scrollPos)
	{
		UINT32 scrollableWidth = (UINT32)std::max(0, INT32(mContentSize.x) - INT32(mVisibleSize.x));
		mHorzOffset = scrollableWidth * Math::Clamp01(scrollPos);

		_markLayoutAsDirty();
	}

	void GUIScrollArea::ScrollToVertical(float pct)
	{
		mVertScroll->_setScrollPos(pct);
		mRecalculateVertOffset = true;

		_markLayoutAsDirty();
	}

	void GUIScrollArea::ScrollToHorizontal(float pct)
	{
		mHorzScroll->_setScrollPos(pct);
		mRecalculateHorzOffset = true;

		_markLayoutAsDirty();
	}

	float GUIScrollArea::GetVerticalScroll() const
	{
		if (mVertScroll != nullptr)
			return mVertScroll->GetScrollPos();

		return 0.0f;
	}

	float GUIScrollArea::GetHorizontalScroll() const
	{
		if (mHorzScroll != nullptr)
			return mHorzScroll->GetScrollPos();

		return 0.0f;
	}

	Rect2I GUIScrollArea::GetContentBounds()
	{
		Rect2I bounds = getBounds();

		if (mHorzScroll)
			bounds.height -= ScrollBarWidth;

		if (mVertScroll)
			bounds.width -= ScrollBarWidth;

		return bounds;
	}

	void GUIScrollArea::ScrollUpPx(UINT32 pixels)
	{
		if(mVertScroll != nullptr)
		{
			UINT32 scrollableSize = (UINT32)std::max(0, INT32(mContentSize.y) - INT32(mVisibleSize.y));

			float offset = 0.0f;
			if(scrollableSize > 0)
				offset = pixels / (float)scrollableSize;

			mVertScroll->Scroll(offset);
		}
	}

	void GUIScrollArea::ScrollDownPx(UINT32 pixels)
	{
		if(mVertScroll != nullptr)
		{
			UINT32 scrollableSize = (UINT32)std::max(0, INT32(mContentSize.y) - INT32(mVisibleSize.y));

			float offset = 0.0f;
			if(scrollableSize > 0)
				offset = pixels / (float)scrollableSize;

			mVertScroll->Scroll(-offset);
		}
	}

	void GUIScrollArea::ScrollLeftPx(UINT32 pixels)
	{
		if(mHorzScroll != nullptr)
		{
			UINT32 scrollableSize = (UINT32)std::max(0, INT32(mContentSize.x) - INT32(mVisibleSize.x));

			float offset = 0.0f;
			if(scrollableSize > 0)
				offset = pixels / (float)scrollableSize;

			mHorzScroll->Scroll(offset);
		}
	}

	void GUIScrollArea::ScrollRightPx(UINT32 pixels)
	{
		if(mHorzScroll != nullptr)
		{
			UINT32 scrollableSize = (UINT32)std::max(0, INT32(mContentSize.x) - INT32(mVisibleSize.x));

			float offset = 0.0f;
			if(scrollableSize > 0)
				offset = pixels / (float)scrollableSize;

			mHorzScroll->Scroll(-offset);
		}
	}

	void GUIScrollArea::ScrollUpPct(float percent)
	{
		if(mVertScroll != nullptr)
			mVertScroll->Scroll(percent);
	}

	void GUIScrollArea::ScrollDownPct(float percent)
	{
		if(mVertScroll != nullptr)
			mVertScroll->Scroll(-percent);
	}

	void GUIScrollArea::ScrollLeftPct(float percent)
	{
		if(mHorzScroll != nullptr)
			mHorzScroll->Scroll(percent);
	}

	void GUIScrollArea::ScrollRightPct(float percent)
	{
		if(mHorzScroll != nullptr)
			mHorzScroll->Scroll(-percent);
	}

	bool GUIScrollArea::_mouseEvent(const GUIMouseEvent& ev)
	{
		if(ev.GetType() == GUIMouseEventType::MouseWheelScroll)
		{
			// Mouse wheel only scrolls on the Y axis
			if(mVertScroll != nullptr)
			{
				UINT32 scrollableHeight = (UINT32)std::max(0, INT32(mContentSize.y) - INT32(mVisibleSize.y));
				float additionalScroll = (float)WheelScrollAmount / scrollableHeight;

				mVertScroll->Scroll(additionalScroll * ev.GetWheelScrollAmount());
				return true;
			}
		}

		return false;
	}

	GUIScrollArea* GUIScrollArea::create(ScrollBarType vertBarType, ScrollBarType horzBarType,
		const String& scrollBarStyle, const String& scrollAreaStyle)
	{
		return new (bs_alloc<GUIScrollArea>()) GUIScrollArea(vertBarType, horzBarType, scrollBarStyle,
			getStyleName<GUIScrollArea>(scrollAreaStyle), GUIDimensions::create());
	}

	GUIScrollArea* GUIScrollArea::create(const GUIOptions& options, const String& scrollBarStyle,
		const String& scrollAreaStyle)
	{
		return new (bs_alloc<GUIScrollArea>()) GUIScrollArea(ScrollBarType::ShowIfDoesntFit,
			ScrollBarType::ShowIfDoesntFit, scrollBarStyle, getStyleName<GUIScrollArea>(scrollAreaStyle), GUIDimensions::create(options));
	}

	GUIScrollArea* GUIScrollArea::create(const String& scrollBarStyle, const String& scrollAreaStyle)
	{
		return new (bs_alloc<GUIScrollArea>()) GUIScrollArea(ScrollBarType::ShowIfDoesntFit, ScrollBarType::ShowIfDoesntFit, scrollBarStyle,
			getStyleName<GUIScrollArea>(scrollAreaStyle), GUIDimensions::create());
	}

	GUIScrollArea* GUIScrollArea::create(ScrollBarType vertBarType,
		ScrollBarType horzBarType, const GUIOptions& options, const String& scrollBarStyle,
		const String& scrollAreaStyle)
	{
		return new (bs_alloc<GUIScrollArea>()) GUIScrollArea(vertBarType, horzBarType, scrollBarStyle,
			getStyleName<GUIScrollArea>(scrollAreaStyle), GUIDimensions::create(options));
	}

	const String& GUIScrollArea::GetGUITypeName()
	{
		static String typeName = "ScrollArea";
		return typeName;
	}
}
