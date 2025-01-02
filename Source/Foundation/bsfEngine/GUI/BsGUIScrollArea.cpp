//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollArea.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIVerticalScrollBar.h"
#include "GUI/BsGUIHorizontalScrollBar.h"
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

	mHorzScroll = GUIHorizontalScrollBar::Create(mScrollBarStyle);
	mVertScroll = GUIVerticalScrollBar::Create(mScrollBarStyle);

	RegisterChildElement(mHorzScroll);
	RegisterChildElement(mVertScroll);

	mHorzScroll->OnScrollOrResize.Connect(std::bind(&GUIScrollArea::HorzScrollUpdate, this, _1));
	mVertScroll->OnScrollOrResize.Connect(std::bind(&GUIScrollArea::VertScrollUpdate, this, _1));
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
	GUIElement::UpdateOptimalLayoutSizes();

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

void GUIScrollArea::GetChildRelativeLayoutAreas(const Size2UI& layoutSize, Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, const GUIConstrainedSize& mySizeRange) const
{
	Vector2I visibleSize, contentSize;
	CalculateRelativeElementAreas(layoutSize, outElementPositions, outElementSizes, elementCount, sizeRanges, visibleSize, contentSize);
}

void GUIScrollArea::CalculateRelativeElementAreas(const Size2UI& scrollAreaSize,  Vector2I* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, Vector2I& visibleSize, Vector2I& contentSize) const
{
	B3D_ASSERT(mChildren.size() == elementCount && elementCount == 3);

	u32 layoutIdx = 0;
	u32 horzScrollIdx = 0;
	u32 vertScrollIdx = 0;
	u32 idx = 0;
	for(auto& child : mChildren) // TODO - Avoid indexing like this here, simply output a struct with relevant information
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
	u32 optimalContentWidth = scrollAreaSize.Width;
	if(mHorzBarType != ScrollBarType::NeverShow)
		optimalContentWidth = sizeRanges[layoutIdx].Optimal.X;

	u32 optimalContentHeight = scrollAreaSize.Height;
	if(mVertBarType != ScrollBarType::NeverShow)
		optimalContentHeight = sizeRanges[layoutIdx].Optimal.Y;

	u32 layoutWidth = std::max(optimalContentWidth, (u32)scrollAreaSize.Width);
	u32 layoutHeight = std::max(optimalContentHeight, (u32)scrollAreaSize.Height);

	contentSize = GUIUtility::CalcActualSize(layoutWidth, layoutHeight, mContentLayout, false);
	visibleSize = Vector2I(scrollAreaSize.Width, scrollAreaSize.Height);

	bool addHorzScrollbar = (mHorzBarType == ScrollBarType::ShowIfDoesntFit && contentSize.X > visibleSize.X) ||
		mHorzBarType == ScrollBarType::AlwaysShow;

	bool hasHorzScrollbar = false;
	bool hasVertScrollbar = false;
	if(addHorzScrollbar)
	{
		// Make room for scrollbar
		visibleSize.Y = (u32)std::max(0, (i32)scrollAreaSize.Height - (i32)kScrollBarWidth);
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
		visibleSize.X = (u32)std::max(0, (i32)scrollAreaSize.Width - (i32)kScrollBarWidth);
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
				visibleSize.Y = (u32)std::max(0, (i32)scrollAreaSize.Height - (i32)kScrollBarWidth);
				optimalContentHeight = (u32)std::max(0, (i32)optimalContentHeight - (i32)kScrollBarWidth);

				if(sizeRanges[layoutIdx].Min.Y > 0)
					optimalContentHeight = std::max((u32)sizeRanges[layoutIdx].Min.Y, optimalContentHeight);

				layoutHeight = std::max(optimalContentHeight, (u32)visibleSize.Y); // Never go below optimal size

				contentSize = GUIUtility::CalcActualSize(layoutWidth, layoutHeight, mContentLayout, true);
				hasHorzScrollbar = true;
			}
		}
	}

	outElementSizes[layoutIdx] = Size2UI(layoutWidth, layoutHeight);
	outElementPositions[layoutIdx] = Vector2I(0, 0);

	// Calculate vertical scrollbar bounds
	if(hasVertScrollbar)
	{
		i32 scrollBarOffset = (u32)std::max(0, (i32)scrollAreaSize.Width - (i32)kScrollBarWidth);
		u32 scrollBarHeight = scrollAreaSize.Height;
		if(hasHorzScrollbar)
			scrollBarHeight = (u32)std::max(0, (i32)scrollBarHeight - (i32)kScrollBarWidth);

		outElementSizes[vertScrollIdx] = Size2UI(kScrollBarWidth, scrollBarHeight);
		outElementPositions[vertScrollIdx] = Vector2I( scrollBarOffset, 0);
	}
	else
	{
		outElementSizes[vertScrollIdx] = Size2UI(0, 0);
		outElementPositions[vertScrollIdx] = Vector2I(layoutWidth, 0);
	}

	// Calculate horizontal scrollbar bounds
	if(hasHorzScrollbar)
	{
		i32 scrollBarOffset = (u32)std::max(0, (i32)scrollAreaSize.Height - (i32)kScrollBarWidth);
		u32 scrollBarWidth = scrollAreaSize.Width;
		if(hasVertScrollbar)
			scrollBarWidth = (u32)std::max(0, (i32)scrollBarWidth - (i32)kScrollBarWidth);

		outElementSizes[horzScrollIdx] = Size2UI(scrollBarWidth, kScrollBarWidth);
		outElementPositions[horzScrollIdx] = Vector2I(0, scrollBarOffset);
	}
	else
	{
		outElementSizes[horzScrollIdx] = Size2UI(0, 0);
		outElementPositions[horzScrollIdx] = Vector2I(0, layoutHeight);
	}
}

void GUIScrollArea::UpdateLayoutRecursive(const GUILayoutData& data)
{
	const u32 elementCount = (u32)mChildren.size();
	Vector2I* elementPositions = nullptr;
	Size2UI* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<Vector2I>(elementCount);
		elementSizes = B3DStackNew<Size2UI>(elementCount);
	}

	u32 layoutIdx = 0;
	u32 horzScrollIdx = 0;
	u32 vertScrollIdx = 0;
	for(u32 i = 0; i < elementCount; i++)
	{
		GUIElement* child = GetChild(i);

		if(child == mContentLayout)
			layoutIdx = i;

		if(child == mHorzScroll)
			horzScrollIdx = i;

		if(child == mVertScroll)
			vertScrollIdx = i;
	}

	const Size2UI scrollAreaSize(data.Size.Width, data.Size.Height);
	CalculateRelativeElementAreas(scrollAreaSize, elementPositions, elementSizes, elementCount, mChildSizeRanges, mVisibleSize, mContentSize);

	// Layout
	if(mContentLayout->IsActive())
	{
		GUILayoutData layoutData = data;
		layoutData.RelativePosition = elementPositions[layoutIdx];
		layoutData.Size = elementSizes[layoutIdx];

		mContentLayout->SetLayoutData(layoutData);
		mContentLayout->UpdateLayoutRecursive(layoutData);
	}

	// Vertical scrollbar
	{
		GUILayoutData layoutData = data;
		layoutData.RelativePosition = elementPositions[vertScrollIdx];
		layoutData.Size = elementSizes[vertScrollIdx];

		mVertScroll->SetLayoutData(layoutData);
		mVertScroll->UpdateLayoutRecursive(layoutData);

		// Set new handle size and update position to match the new size
		u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(layoutData.Size.Height));
		float newScrollPct = 0.0f;

		if(scrollableHeight > 0)
			newScrollPct = mVertOffset / scrollableHeight;

		mVertScroll->SetHandleSizeInternal(layoutData.Size.Height / (float)mContentSize.Y);
		mVertScroll->SetScrollPosInternal(newScrollPct);
	}

	// Horizontal scrollbar
	{
		GUILayoutData layoutData = data;
		layoutData.RelativePosition = elementPositions[horzScrollIdx];
		layoutData.Size = elementSizes[horzScrollIdx];

		mHorzScroll->SetLayoutData(layoutData);
		mHorzScroll->UpdateLayoutRecursive(layoutData);

		// Set new handle size and update position to match the new size
		u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(layoutData.Size.Width));
		float newScrollPct = 0.0f;

		if(scrollableWidth > 0)
			newScrollPct = mHorzOffset / scrollableWidth;

		mHorzScroll->SetHandleSizeInternal(layoutData.Size.Width / (float)mContentSize.X);
		mHorzScroll->SetScrollPosInternal(newScrollPct);
	}

	if(elementSizes != nullptr)
		B3DStackFree(elementSizes);

	if(elementPositions != nullptr)
		B3DStackFree(elementPositions);
}

void GUIScrollArea::UpdateAbsoluteCoordinatesForChildren()
{
	// Recalculate offsets in case scroll percent got updated externally (this needs to be delayed to this point because
	// at the time of the update content and visible sizes might be out of date).
	u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
	if(mRecalculateVertOffset)
	{
		mVertOffset = (float)scrollableHeight * Math::Clamp01(mVertScroll->GetScrollHandlePosition());
		mRecalculateVertOffset = false;
	}

	u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));
	if(mRecalculateHorzOffset)
	{
		mHorzOffset = (float)scrollableWidth * Math::Clamp01(mHorzScroll->GetScrollHandlePosition());
		mRecalculateHorzOffset = false;
	}

	// Reset offset in case layout size changed so everything fits
	mVertOffset = Math::Clamp(mVertOffset, 0.0f, (float)scrollableHeight);
	mHorzOffset = Math::Clamp(mHorzOffset, 0.0f, (float)scrollableWidth);

	if(mContentLayout->IsActive())
	{
		const Vector2I contentOrigin(mAbsolutePosition.X - Math::FloorToInt(mHorzOffset), mAbsolutePosition.Y - Math::FloorToInt(mVertOffset));
		const Rect2I contentVisibleAreaSize(mAbsolutePosition.X, mAbsolutePosition.Y, (u32)mVisibleSize.X, (u32)mVisibleSize.Y); // TODO - Clip visible size by parent clip rectangle?

		mContentLayout->UpdateAbsoluteCoordinates(contentOrigin, contentVisibleAreaSize);
	}

	mHorzScroll->UpdateAbsoluteCoordinates(mAbsolutePosition, mAbsoluteClippedArea);
	mVertScroll->UpdateAbsoluteCoordinates(mAbsolutePosition, mAbsoluteClippedArea);

	// TODO - Need to mark elements as culled/not culled and add/remove them from widget draw group. Elements should by default
	// not be added to the widget draw group on registration
	// - Skip element if not visible
	// - Also in various places where we check IsVisible, we need to check IsCulled
}

void GUIScrollArea::VertScrollUpdate(float scrollPos)
{
	u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
	mVertOffset = scrollableHeight * Math::Clamp01(scrollPos);

	MarkLayoutAsDirty(); // TODO - Don't dirty whole layout. Add a method that marks visible area as dirty, then the widget will take care of updating it
}

void GUIScrollArea::HorzScrollUpdate(float scrollPos)
{
	u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));
	mHorzOffset = scrollableWidth * Math::Clamp01(scrollPos);

	MarkLayoutAsDirty(); // TODO - Don't dirty whole layout
}

void GUIScrollArea::ScrollToVertical(float pct)
{
	mVertScroll->SetScrollPosInternal(pct);
	mRecalculateVertOffset = true;

	MarkLayoutAsDirty(); // TODO - Don't dirty whole layout
}

void GUIScrollArea::ScrollToHorizontal(float pct)
{
	mHorzScroll->SetScrollPosInternal(pct);
	mRecalculateHorzOffset = true;

	MarkLayoutAsDirty(); // TODO - Don't dirty whole layout
}

float GUIScrollArea::GetVerticalScroll() const
{
	if(mVertScroll != nullptr)
		return mVertScroll->GetScrollHandlePosition();

	return 0.0f;
}

float GUIScrollArea::GetHorizontalScroll() const
{
	if(mHorzScroll != nullptr)
		return mHorzScroll->GetScrollHandlePosition();

	return 0.0f;
}

Rect2I GUIScrollArea::GetContentBounds()
{
	Rect2I bounds = CalculateBoundsRelativeTo();

	if(mHorzScroll)
		bounds.Height -= kScrollBarWidth;

	if(mVertScroll)
		bounds.Width -= kScrollBarWidth;

	return bounds;
}

void GUIScrollArea::ScrollUpPixels(u32 pixels)
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

void GUIScrollArea::ScrollDownPixels(u32 pixels)
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

void GUIScrollArea::ScrollLeftPixels(u32 pixels)
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

void GUIScrollArea::ScrollRightPixels(u32 pixels)
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

void GUIScrollArea::ScrollUpPercent(float percent)
{
	if(mVertScroll != nullptr)
		mVertScroll->Scroll(percent);
}

void GUIScrollArea::ScrollDownPercent(float percent)
{
	if(mVertScroll != nullptr)
		mVertScroll->Scroll(-percent);
}

void GUIScrollArea::ScrollLeftPercent(float percent)
{
	if(mHorzScroll != nullptr)
		mHorzScroll->Scroll(percent);
}

void GUIScrollArea::ScrollRightPercent(float percent)
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
