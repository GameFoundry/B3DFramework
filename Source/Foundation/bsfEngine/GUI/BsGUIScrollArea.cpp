//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollArea.h"
#include "GUI/BsGUIElementStyle.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIScrollBarVert.h"
#include "GUI/BsGUIScrollBarHorz.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIUtility.h"

using namespace std::placeholders;

using namespace bs;

const u32 GUIScrollArea::kScrollBarWidth = 16;
const u32 GUIScrollArea::kWheelScrollAmount = 50;

GUIScrollArea::GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType, const String& scrollBarStyle, const String& scrollAreaStyle, const GUISizeConstraints& dimensions)
	: GUIElementContainer(dimensions), mVertBarType(vertBarType), mHorzBarType(horzBarType), mScrollBarStyle(scrollBarStyle), mVertScroll(nullptr), mHorzScroll(nullptr), mVertOffset(0), mHorzOffset(0), mRecalculateVertOffset(false), mRecalculateHorzOffset(false)
{
	mContentLayout = GUILayoutY::Create();
	RegisterChildElement(mContentLayout);

	mHorzScroll = GUIScrollBarHorz::Create(mScrollBarStyle);
	mVertScroll = GUIScrollBarVert::Create(mScrollBarStyle);

	RegisterChildElement(mHorzScroll);
	RegisterChildElement(mVertScroll);

	mHorzScroll->OnScrollOrResize.Connect(std::bind(&GUIScrollArea::HorzScrollUpdate, this, _1));
	mVertScroll->OnScrollOrResize.Connect(std::bind(&GUIScrollArea::VertScrollUpdate, this, _1));
}

void GUIScrollArea::UpdateClippedBounds()
{
	mClippedBounds = mLayoutData.Area;
	mClippedBounds.Clip(mLayoutData.ClipRect);
}

Vector2I GUIScrollArea::CalculateUnconstrainedOptimalSize() const
{
	Vector2I optimalSize = mContentLayout->CalculateUnconstrainedOptimalSize();

	// Provide 10x10 in case underlying layout is empty because
	// 0 doesn't work well with the layout system
	optimalSize.X = std::max(10, optimalSize.X);
	optimalSize.Y = std::max(10, optimalSize.Y);

	return optimalSize;
}

GUIConstrainedSize GUIScrollArea::CalculateConstrainedSize() const
{
	// I'm ignoring scroll bars here since if the content layout fits
	// then they're not needed and the range is valid. And if it doesn't
	// fit the area will get clipped anyway and including the scroll bars
	// won't change the size much, but it would complicate this method significantly.
	if(mContentLayout->IsActive())
		return mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize());

	return mSizeConstraints.CalculateConstrainedSize(Vector2I());
}

GUIConstrainedSize GUIScrollArea::GetConstrainedSize() const
{
	return mSizeRange;
}

void GUIScrollArea::UpdateOptimalLayoutSizes()
{
	// Update all children first, otherwise we can't determine our own optimal size
	GUIElementBase::UpdateOptimalLayoutSizes();

	if(mChildren.size() != mChildSizeRanges.size())
		mChildSizeRanges.resize(mChildren.size());

	u32 childIdx = 0;
	for(auto& child : mChildren)
	{
		if(child->IsActive())
			mChildSizeRanges[childIdx] = child->GetConstrainedSize();
		else
			mChildSizeRanges[childIdx] = GUIConstrainedSize();

		childIdx++;
	}

	mSizeRange = mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize());
}

void GUIScrollArea::GetChildLayoutAreas(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const
{
	Vector2I visibleSize, contentSize;
	GetElementAreasInternal(layoutArea, elementAreas, numElements, sizeRanges, visibleSize, contentSize);
}

void GUIScrollArea::GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<GUIConstrainedSize>& sizeRanges, Vector2I& visibleSize, Vector2I& contentSize) const
{
	B3D_ASSERT(mChildren.size() == numElements && numElements == 3);

	u32 layoutIdx = 0;
	u32 horzScrollIdx = 0;
	u32 vertScrollIdx = 0;
	u32 idx = 0;
	for(auto& child : mChildren)
	{
		if(child == mContentLayout)
			layoutIdx = idx;

		if(child == mHorzScroll)
			horzScrollIdx = idx;

		if(child == mVertScroll)
			vertScrollIdx = idx;

		idx++;
	}

	// Calculate content layout bounds

	//// We want elements to use their optimal height, since scroll area
	//// technically provides "infinite" space
	u32 optimalContentWidth = layoutArea.Width;
	if(mHorzBarType != ScrollBarType::NeverShow)
		optimalContentWidth = sizeRanges[layoutIdx].Optimal.X;

	u32 optimalContentHeight = layoutArea.Height;
	if(mVertBarType != ScrollBarType::NeverShow)
		optimalContentHeight = sizeRanges[layoutIdx].Optimal.Y;

	u32 layoutWidth = std::max(optimalContentWidth, (u32)layoutArea.Width);
	u32 layoutHeight = std::max(optimalContentHeight, (u32)layoutArea.Height);

	contentSize = GUIUtility::CalcActualSize(layoutWidth, layoutHeight, mContentLayout, false);
	visibleSize = Vector2I(layoutArea.Width, layoutArea.Height);

	bool addHorzScrollbar = (mHorzBarType == ScrollBarType::ShowIfDoesntFit && contentSize.X > visibleSize.X) ||
		mHorzBarType == ScrollBarType::AlwaysShow;

	bool hasHorzScrollbar = false;
	bool hasVertScrollbar = false;
	if(addHorzScrollbar)
	{
		// Make room for scrollbar
		visibleSize.Y = (u32)std::max(0, (i32)layoutArea.Height - (i32)kScrollBarWidth);
		optimalContentHeight = (u32)std::max(0, (i32)optimalContentHeight - (i32)kScrollBarWidth);

		if(sizeRanges[layoutIdx].Min.Y > 0)
			optimalContentHeight = std::max((u32)sizeRanges[layoutIdx].Min.Y, optimalContentHeight);

		layoutHeight = std::max(optimalContentHeight, (u32)visibleSize.Y); // Never go below optimal size

		contentSize = GUIUtility::CalcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
		hasHorzScrollbar = true;
	}

	bool addVertScrollbar = (mVertBarType == ScrollBarType::ShowIfDoesntFit && contentSize.Y > visibleSize.Y) ||
		mVertBarType == ScrollBarType::AlwaysShow;

	if(addVertScrollbar)
	{
		// Make room for scrollbar
		visibleSize.X = (u32)std::max(0, (i32)layoutArea.Width - (i32)kScrollBarWidth);
		optimalContentWidth = (u32)std::max(0, (i32)optimalContentWidth - (i32)kScrollBarWidth);

		if(sizeRanges[layoutIdx].Min.X > 0)
			optimalContentWidth = std::max((u32)sizeRanges[layoutIdx].Min.X, optimalContentWidth);

		layoutWidth = std::max(optimalContentWidth, (u32)visibleSize.X); // Never go below optimal size

		contentSize = GUIUtility::CalcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
		hasVertScrollbar = true;

		if(!hasHorzScrollbar) // Since width has been reduced, we need to check if we require the horizontal scrollbar
		{
			addHorzScrollbar = (mHorzBarType == ScrollBarType::ShowIfDoesntFit && contentSize.X > visibleSize.X) && mHorzBarType != ScrollBarType::NeverShow;

			if(addHorzScrollbar)
			{
				// Make room for scrollbar
				visibleSize.Y = (u32)std::max(0, (i32)layoutArea.Height - (i32)kScrollBarWidth);
				optimalContentHeight = (u32)std::max(0, (i32)optimalContentHeight - (i32)kScrollBarWidth);

				if(sizeRanges[layoutIdx].Min.Y > 0)
					optimalContentHeight = std::max((u32)sizeRanges[layoutIdx].Min.Y, optimalContentHeight);

				layoutHeight = std::max(optimalContentHeight, (u32)visibleSize.Y); // Never go below optimal size

				contentSize = GUIUtility::CalcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
				hasHorzScrollbar = true;
			}
		}
	}

	elementAreas[layoutIdx] = Rect2I(layoutArea.X, layoutArea.Y, layoutWidth, layoutHeight);

	// Calculate vertical scrollbar bounds
	if(hasVertScrollbar)
	{
		i32 scrollBarOffset = (u32)std::max(0, (i32)layoutArea.Width - (i32)kScrollBarWidth);
		u32 scrollBarHeight = layoutArea.Height;
		if(hasHorzScrollbar)
			scrollBarHeight = (u32)std::max(0, (i32)scrollBarHeight - (i32)kScrollBarWidth);

		elementAreas[vertScrollIdx] = Rect2I(layoutArea.X + scrollBarOffset, layoutArea.Y, kScrollBarWidth, scrollBarHeight);
	}
	else
	{
		elementAreas[vertScrollIdx] = Rect2I(layoutArea.X + layoutWidth, layoutArea.Y, 0, 0);
	}

	// Calculate horizontal scrollbar bounds
	if(hasHorzScrollbar)
	{
		i32 scrollBarOffset = (u32)std::max(0, (i32)layoutArea.Height - (i32)kScrollBarWidth);
		u32 scrollBarWidth = layoutArea.Width;
		if(hasVertScrollbar)
			scrollBarWidth = (u32)std::max(0, (i32)scrollBarWidth - (i32)kScrollBarWidth);

		elementAreas[horzScrollIdx] = Rect2I(layoutArea.X, layoutArea.Y + scrollBarOffset, scrollBarWidth, kScrollBarWidth);
	}
	else
	{
		elementAreas[horzScrollIdx] = Rect2I(layoutArea.X, layoutArea.Y + layoutHeight, 0, 0);
	}
}

void GUIScrollArea::UpdateLayoutRecursive(const GUILayoutData& data)
{
	u32 numElements = (u32)mChildren.size();
	Rect2I* elementAreas = nullptr;

	if(numElements > 0)
		elementAreas = B3DStackNew<Rect2I>(numElements);

	u32 layoutIdx = 0;
	u32 horzScrollIdx = 0;
	u32 vertScrollIdx = 0;
	for(u32 i = 0; i < numElements; i++)
	{
		GUIElementBase* child = GetChild(i);

		if(child == mContentLayout)
			layoutIdx = i;

		if(child == mHorzScroll)
			horzScrollIdx = i;

		if(child == mVertScroll)
			vertScrollIdx = i;
	}

	GetElementAreasInternal(data.Area, elementAreas, numElements, mChildSizeRanges, mVisibleSize, mContentSize);

	Rect2I& layoutBounds = elementAreas[layoutIdx];
	Rect2I& horzScrollBounds = elementAreas[horzScrollIdx];
	Rect2I& vertScrollBounds = elementAreas[vertScrollIdx];

	// Recalculate offsets in case scroll percent got updated externally (this needs to be delayed to this point because
	// at the time of the update content and visible sizes might be out of date).
	u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
	if(mRecalculateVertOffset)
	{
		mVertOffset = scrollableHeight * Math::Clamp01(mVertScroll->GetScrollPos());

		mRecalculateVertOffset = false;
	}

	u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));
	if(mRecalculateHorzOffset)
	{
		mHorzOffset = scrollableWidth * Math::Clamp01(mHorzScroll->GetScrollPos());

		mRecalculateHorzOffset = false;
	}

	// Reset offset in case layout size changed so everything fits
	mVertOffset = Math::Clamp(mVertOffset, 0.0f, (float)scrollableHeight);
	mHorzOffset = Math::Clamp(mHorzOffset, 0.0f, (float)scrollableWidth);

	// Layout
	if(mContentLayout->IsActive())
	{
		layoutBounds.X -= Math::FloorToInt(mHorzOffset);
		layoutBounds.Y -= Math::FloorToInt(mVertOffset);

		Rect2I layoutClipRect = data.ClipRect;
		layoutClipRect.Width = (u32)mVisibleSize.X;
		layoutClipRect.Height = (u32)mVisibleSize.Y;
		layoutClipRect.Clip(data.ClipRect);

		GUILayoutData layoutData = data;
		layoutData.Area = layoutBounds;
		layoutData.ClipRect = layoutClipRect;

		mContentLayout->SetLayoutData(layoutData);
		mContentLayout->UpdateLayoutRecursive(layoutData);
	}

	// Vertical scrollbar
	{
		GUILayoutData vertScrollData = data;
		vertScrollData.Area = vertScrollBounds;

		vertScrollData.ClipRect = vertScrollBounds;
		vertScrollData.ClipRect.Clip(data.ClipRect);

		mVertScroll->SetLayoutData(vertScrollData);
		mVertScroll->UpdateLayoutRecursive(vertScrollData);

		// Set new handle size and update position to match the new size
		u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(vertScrollBounds.Height));
		float newScrollPct = 0.0f;

		if(scrollableHeight > 0)
			newScrollPct = mVertOffset / scrollableHeight;

		mVertScroll->SetHandleSizeInternal(vertScrollBounds.Height / (float)mContentSize.Y);
		mVertScroll->SetScrollPosInternal(newScrollPct);
	}

	// Horizontal scrollbar
	{
		GUILayoutData horzScrollData = data;
		horzScrollData.Area = horzScrollBounds;

		horzScrollData.ClipRect = horzScrollBounds;
		horzScrollData.ClipRect.Clip(data.ClipRect);

		mHorzScroll->SetLayoutData(horzScrollData);
		mHorzScroll->UpdateLayoutRecursive(horzScrollData);

		// Set new handle size and update position to match the new size
		u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(horzScrollBounds.Width));
		float newScrollPct = 0.0f;

		if(scrollableWidth > 0)
			newScrollPct = mHorzOffset / scrollableWidth;

		mHorzScroll->SetHandleSizeInternal(horzScrollBounds.Width / (float)mContentSize.X);
		mHorzScroll->SetScrollPosInternal(newScrollPct);
	}

	if(elementAreas != nullptr)
		B3DStackFree(elementAreas);
}

void GUIScrollArea::VertScrollUpdate(float scrollPos)
{
	u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
	mVertOffset = scrollableHeight * Math::Clamp01(scrollPos);

	MarkLayoutAsDirty();
}

void GUIScrollArea::HorzScrollUpdate(float scrollPos)
{
	u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));
	mHorzOffset = scrollableWidth * Math::Clamp01(scrollPos);

	MarkLayoutAsDirty();
}

void GUIScrollArea::ScrollToVertical(float pct)
{
	mVertScroll->SetScrollPosInternal(pct);
	mRecalculateVertOffset = true;

	MarkLayoutAsDirty();
}

void GUIScrollArea::ScrollToHorizontal(float pct)
{
	mHorzScroll->SetScrollPosInternal(pct);
	mRecalculateHorzOffset = true;

	MarkLayoutAsDirty();
}

float GUIScrollArea::GetVerticalScroll() const
{
	if(mVertScroll != nullptr)
		return mVertScroll->GetScrollPos();

	return 0.0f;
}

float GUIScrollArea::GetHorizontalScroll() const
{
	if(mHorzScroll != nullptr)
		return mHorzScroll->GetScrollPos();

	return 0.0f;
}

Rect2I GUIScrollArea::GetContentBounds()
{
	Rect2I bounds = GetBoundsRelativeTo();

	if(mHorzScroll)
		bounds.Height -= kScrollBarWidth;

	if(mVertScroll)
		bounds.Width -= kScrollBarWidth;

	return bounds;
}

void GUIScrollArea::ScrollUpPx(u32 pixels)
{
	if(mVertScroll != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mVertScroll->Scroll(offset);
	}
}

void GUIScrollArea::ScrollDownPx(u32 pixels)
{
	if(mVertScroll != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mVertScroll->Scroll(-offset);
	}
}

void GUIScrollArea::ScrollLeftPx(u32 pixels)
{
	if(mHorzScroll != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mHorzScroll->Scroll(offset);
	}
}

void GUIScrollArea::ScrollRightPx(u32 pixels)
{
	if(mHorzScroll != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));

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

bool GUIScrollArea::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseWheelScroll)
	{
		// Mouse wheel only scrolls on the Y axis
		if(mVertScroll != nullptr)
		{
			u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
			float additionalScroll = (float)kWheelScrollAmount / scrollableHeight;

			mVertScroll->Scroll(additionalScroll * ev.GetWheelScrollAmount());
			return true;
		}
	}

	return false;
}

GUIScrollArea* GUIScrollArea::Create(ScrollBarType vertBarType, ScrollBarType horzBarType, const String& scrollBarStyle, const String& scrollAreaStyle)
{
	return new(B3DAllocate<GUIScrollArea>()) GUIScrollArea(vertBarType, horzBarType, scrollBarStyle, GetStyleClass<GUIScrollArea>(scrollAreaStyle), GUISizeConstraints::Create());
}

GUIScrollArea* GUIScrollArea::Create(const GUIOptions& options, const String& scrollBarStyle, const String& scrollAreaStyle)
{
	return new(B3DAllocate<GUIScrollArea>()) GUIScrollArea(ScrollBarType::ShowIfDoesntFit, ScrollBarType::ShowIfDoesntFit, scrollBarStyle, GetStyleClass<GUIScrollArea>(scrollAreaStyle), GUISizeConstraints::Create(options));
}

GUIScrollArea* GUIScrollArea::Create(const String& scrollBarStyle, const String& scrollAreaStyle)
{
	return new(B3DAllocate<GUIScrollArea>()) GUIScrollArea(ScrollBarType::ShowIfDoesntFit, ScrollBarType::ShowIfDoesntFit, scrollBarStyle, GetStyleClass<GUIScrollArea>(scrollAreaStyle), GUISizeConstraints::Create());
}

GUIScrollArea* GUIScrollArea::Create(ScrollBarType vertBarType, ScrollBarType horzBarType, const GUIOptions& options, const String& scrollBarStyle, const String& scrollAreaStyle)
{
	return new(B3DAllocate<GUIScrollArea>()) GUIScrollArea(vertBarType, horzBarType, scrollBarStyle, GetStyleClass<GUIScrollArea>(scrollAreaStyle), GUISizeConstraints::Create(options));
}

const String& GUIScrollArea::GetGuiTypeName()
{
	static String typeName = "ScrollArea";
	return typeName;
}
