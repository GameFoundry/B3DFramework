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
	: GUIElementContainer(dimensions, scrollAreaStyle), mVerticalScrollBarType(vertBarType), mHorizontalScrollBarType(horzBarType), mScrollBarStyle(scrollBarStyle), mVerticalScrollBar(nullptr), mHorizontalScrollBar(nullptr), mVertOffset(0), mHorzOffset(0), mRecalculateVertOffset(false), mRecalculateHorzOffset(false)
{
	mContentLayout = GUILayoutY::Create();
	RegisterChildElement(mContentLayout);

	mHorizontalScrollBar = GUIHorizontalScrollBar::Create(mScrollBarStyle);
	mVerticalScrollBar = GUIVerticalScrollBar::Create(mScrollBarStyle);

	RegisterChildElement(mHorizontalScrollBar);
	RegisterChildElement(mVerticalScrollBar);

	mHorizontalScrollBar->OnScrollOrResize.Connect(std::bind(&GUIScrollArea::HorzScrollUpdate, this, _1));
	mVerticalScrollBar->OnScrollOrResize.Connect(std::bind(&GUIScrollArea::VertScrollUpdate, this, _1));
}

Vector2I GUIScrollArea::CalculateUnconstrainedOptimalSize() const
{
	// TODO - For layouts the function call below actually returns constrained size, despite the name
	Vector2I optimalSize = mContentLayout->CalculateUnconstrainedOptimalSize();

	if(mVerticalScrollBarType != ScrollBarType::NeverShow)
		optimalSize.X += kScrollBarWidth;

	if(mHorizontalScrollBarType != ScrollBarType::NeverShow)
		optimalSize.Y += kScrollBarWidth;

	// Provide 10x10 in case underlying layout is empty because
	// 0 doesn't work well with the layout system
	optimalSize.X = std::max(10, optimalSize.X);
	optimalSize.Y = std::max(10, optimalSize.Y);

	return optimalSize;
}

GUIConstrainedSize GUIScrollArea::CalculateConstrainedSize() const
{
	if(mContentLayout->IsActive())
		return mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize());

	return mSizeConstraints.CalculateConstrainedSize(Vector2I(BsZero));
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

void GUIScrollArea::CalculateRelativeElementAreas(const Size2UI& scrollAreaSize,  GUILogicalPoint* outElementPositions, Size2UI* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, Vector2I& outVisibleSize) const
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

		if(child == mHorizontalScrollBar)
			horzScrollIdx = idx;

		if(child == mVerticalScrollBar)
			vertScrollIdx = idx;

		idx++;
	}

	// Calculate content layout bounds

	//// We want elements to use their optimal height, since scroll area
	//// technically provides "infinite" space
	u32 optimalContentWidth = scrollAreaSize.Width;
	if(mHorizontalScrollBarType != ScrollBarType::NeverShow)
		optimalContentWidth = sizeRanges[layoutIdx].Optimal.X;

	u32 optimalContentHeight = scrollAreaSize.Height;
	if(mVerticalScrollBarType != ScrollBarType::NeverShow)
		optimalContentHeight = sizeRanges[layoutIdx].Optimal.Y;

	u32 layoutWidth = std::max(optimalContentWidth, (u32)scrollAreaSize.Width);
	u32 layoutHeight = std::max(optimalContentHeight, (u32)scrollAreaSize.Height);

	outVisibleSize = Vector2I(scrollAreaSize.Width, scrollAreaSize.Height);

	const bool hasHorizontalScrollbar = mHorizontalScrollBarType != ScrollBarType::NeverShow;
	const bool hasVerticalScrollbar = mVerticalScrollBarType != ScrollBarType::NeverShow;
	if(hasHorizontalScrollbar)
	{
		// Make room for scrollbar
		outVisibleSize.Y = (u32)std::max(0, (i32)scrollAreaSize.Height - (i32)kScrollBarWidth);
		optimalContentHeight = (u32)std::max(0, (i32)optimalContentHeight - (i32)kScrollBarWidth);

		if(sizeRanges[layoutIdx].Min.Y > 0)
			optimalContentHeight = std::max((u32)sizeRanges[layoutIdx].Min.Y, optimalContentHeight);

		layoutHeight = std::max(optimalContentHeight, (u32)outVisibleSize.Y); // Never go below optimal size
	}

	if(hasVerticalScrollbar)
	{
		// Make room for scrollbar
		outVisibleSize.X = (u32)std::max(0, (i32)scrollAreaSize.Width - (i32)kScrollBarWidth);
		optimalContentWidth = (u32)std::max(0, (i32)optimalContentWidth - (i32)kScrollBarWidth);

		if(sizeRanges[layoutIdx].Min.X > 0)
			optimalContentWidth = std::max((u32)sizeRanges[layoutIdx].Min.X, optimalContentWidth);

		layoutWidth = std::max(optimalContentWidth, (u32)outVisibleSize.X); // Never go below optimal size
	}

	outElementSizes[layoutIdx] = Size2UI(layoutWidth, layoutHeight);
	outElementPositions[layoutIdx] = GUILogicalPoint(0, 0);

	// Calculate vertical scrollbar bounds
	if(hasVerticalScrollbar)
	{
		i32 scrollBarOffset = (u32)std::max(0, (i32)scrollAreaSize.Width - (i32)kScrollBarWidth);
		u32 scrollBarHeight = scrollAreaSize.Height;
		if(hasHorizontalScrollbar)
			scrollBarHeight = (u32)std::max(0, (i32)scrollBarHeight - (i32)kScrollBarWidth);

		outElementSizes[vertScrollIdx] = Size2UI(kScrollBarWidth, scrollBarHeight);
		outElementPositions[vertScrollIdx] = GUILogicalPoint( scrollBarOffset, 0);
	}
	else
	{
		outElementSizes[vertScrollIdx] = Size2UI(0, 0);
		outElementPositions[vertScrollIdx] = GUILogicalPoint(layoutWidth, 0);
	}

	// Calculate horizontal scrollbar bounds
	if(hasHorizontalScrollbar)
	{
		i32 scrollBarOffset = (u32)std::max(0, (i32)scrollAreaSize.Height - (i32)kScrollBarWidth);
		u32 scrollBarWidth = scrollAreaSize.Width;
		if(hasVerticalScrollbar)
			scrollBarWidth = (u32)std::max(0, (i32)scrollBarWidth - (i32)kScrollBarWidth);

		outElementSizes[horzScrollIdx] = Size2UI(scrollBarWidth, kScrollBarWidth);
		outElementPositions[horzScrollIdx] = GUILogicalPoint(0, scrollBarOffset);
	}
	else
	{
		outElementSizes[horzScrollIdx] = Size2UI(0, 0);
		outElementPositions[horzScrollIdx] = GUILogicalPoint(0, layoutHeight);
	}
}

void GUIScrollArea::UpdateLayoutForChildren()
{
	const u32 elementCount = (u32)mChildren.size();
	GUILogicalPoint* elementPositions = nullptr;
	Size2UI* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<GUILogicalPoint>(elementCount);
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

		if(child == mHorizontalScrollBar)
			horzScrollIdx = i;

		if(child == mVerticalScrollBar)
			vertScrollIdx = i;
	}

	const Size2UI scrollAreaSize(mLayoutData.Size.Width, mLayoutData.Size.Height);
	CalculateRelativeElementAreas(scrollAreaSize, elementPositions, elementSizes, elementCount, mChildSizeRanges, mVisibleSize);

	// Layout
	if(mContentLayout->IsActive())
	{
		GUILayoutData layoutData = mLayoutData;
		layoutData.RelativePosition = elementPositions[layoutIdx];
		layoutData.Size = elementSizes[layoutIdx];

		mContentLayout->SetLayoutData(layoutData);
		mContentLayout->UpdateLayoutForChildren();
	}

	mContentSize = Vector2I((i32)elementSizes[layoutIdx].Width, (i32)elementSizes[layoutIdx].Height);

	const bool showVerticalScrollBar = mVerticalScrollBarType != ScrollBarType::NeverShow && mContentSize.Y > mVisibleSize.Y;
	const bool showHorizontalScrollBar = mHorizontalScrollBarType != ScrollBarType::NeverShow && mContentSize.X > mVisibleSize.X;

	// Vertical scrollbar
	{
		GUILayoutData layoutData = mLayoutData;
		layoutData.RelativePosition = elementPositions[vertScrollIdx];

		if(showVerticalScrollBar)
			layoutData.Size = elementSizes[vertScrollIdx];
		else
			layoutData.Size = Size2UI(0u, 0u);

		mVerticalScrollBar->SetLayoutData(layoutData);
		mVerticalScrollBar->UpdateLayoutForChildren();

		// Set new handle size and update position to match the new size
		u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(layoutData.Size.Height));
		float newScrollPct = 0.0f;

		if(scrollableHeight > 0)
			newScrollPct = mVertOffset / scrollableHeight;

		mVerticalScrollBar->SetHandleSizeInternal(layoutData.Size.Height / (float)mContentSize.Y);
		mVerticalScrollBar->SetScrollPosInternal(newScrollPct);
	}

	// Horizontal scrollbar
	{
		GUILayoutData layoutData = mLayoutData;
		layoutData.RelativePosition = elementPositions[horzScrollIdx];

		if(showHorizontalScrollBar)
			layoutData.Size = elementSizes[horzScrollIdx];
		else
			layoutData.Size = Size2UI(0u, 0u);

		mHorizontalScrollBar->SetLayoutData(layoutData);
		mHorizontalScrollBar->UpdateLayoutForChildren();

		// Set new handle size and update position to match the new size
		u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(layoutData.Size.Width));
		float newScrollPct = 0.0f;

		if(scrollableWidth > 0)
			newScrollPct = mHorzOffset / scrollableWidth;

		mHorizontalScrollBar->SetHandleSizeInternal(layoutData.Size.Width / (float)mContentSize.X);
		mHorizontalScrollBar->SetScrollPosInternal(newScrollPct);
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
		mVertOffset = (float)scrollableHeight * Math::Clamp01(mVerticalScrollBar->GetScrollHandlePosition());
		mRecalculateVertOffset = false;
	}

	u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));
	if(mRecalculateHorzOffset)
	{
		mHorzOffset = (float)scrollableWidth * Math::Clamp01(mHorizontalScrollBar->GetScrollHandlePosition());
		mRecalculateHorzOffset = false;
	}

	// Reset offset in case layout size changed so everything fits
	mVertOffset = Math::Clamp(mVertOffset, 0.0f, (float)scrollableHeight);
	mHorzOffset = Math::Clamp(mHorzOffset, 0.0f, (float)scrollableWidth);

	if(mContentLayout->IsActive())
	{
		const Vector2I contentOrigin(mAbsolutePosition.X - Math::FloorToInt(mHorzOffset * mAbsoluteScale), mAbsolutePosition.Y - Math::FloorToInt(mVertOffset * mAbsoluteScale));
		const Rect2I contentVisibleAreaSize(mAbsolutePosition.X, mAbsolutePosition.Y, (u32)((float)mVisibleSize.X * mAbsoluteScale), (u32)((float)mVisibleSize.Y * mAbsoluteScale)); // TODO - Clip visible size by parent clip rectangle?

		mContentLayout->UpdateAbsoluteCoordinates(contentOrigin, mAbsoluteScale, contentVisibleAreaSize);
	}

	mHorizontalScrollBar->UpdateAbsoluteCoordinates(mAbsolutePosition, mAbsoluteScale, mAbsoluteClippedArea);
	mVerticalScrollBar->UpdateAbsoluteCoordinates(mAbsolutePosition, mAbsoluteScale, mAbsoluteClippedArea);
}

void GUIScrollArea::VertScrollUpdate(float scrollPos)
{
	u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
	mVertOffset = scrollableHeight * Math::Clamp01(scrollPos);

	MarkAbsoluteCoordinatesAsDirty();
}

void GUIScrollArea::HorzScrollUpdate(float scrollPos)
{
	u32 scrollableWidth = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));
	mHorzOffset = scrollableWidth * Math::Clamp01(scrollPos);

	MarkAbsoluteCoordinatesAsDirty();
}

void GUIScrollArea::ScrollToVertical(float pct)
{
	mVerticalScrollBar->SetScrollPosInternal(pct);
	mRecalculateVertOffset = true;

	MarkAbsoluteCoordinatesAsDirty();
}

void GUIScrollArea::ScrollToHorizontal(float pct)
{
	mHorizontalScrollBar->SetScrollPosInternal(pct);
	mRecalculateHorzOffset = true;

	MarkAbsoluteCoordinatesAsDirty();
}

float GUIScrollArea::GetVerticalScroll() const
{
	if(mVerticalScrollBar != nullptr)
		return mVerticalScrollBar->GetScrollHandlePosition();

	return 0.0f;
}

float GUIScrollArea::GetHorizontalScroll() const
{
	if(mHorizontalScrollBar != nullptr)
		return mHorizontalScrollBar->GetScrollHandlePosition();

	return 0.0f;
}

Rect2I GUIScrollArea::GetContentBounds()
{
	Rect2I bounds = CalculateAbsoluteBoundsRelativeTo();

	if(mHorizontalScrollBar)
		bounds.Height -= kScrollBarWidth;

	if(mVerticalScrollBar)
		bounds.Width -= kScrollBarWidth;

	return bounds;
}

void GUIScrollArea::ScrollUpPixels(u32 pixels)
{
	if(mVerticalScrollBar != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mVerticalScrollBar->Scroll(offset);
	}
}

void GUIScrollArea::ScrollDownPixels(u32 pixels)
{
	if(mVerticalScrollBar != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mVerticalScrollBar->Scroll(-offset);
	}
}

void GUIScrollArea::ScrollLeftPixels(u32 pixels)
{
	if(mHorizontalScrollBar != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mHorizontalScrollBar->Scroll(offset);
	}
}

void GUIScrollArea::ScrollRightPixels(u32 pixels)
{
	if(mHorizontalScrollBar != nullptr)
	{
		u32 scrollableSize = (u32)std::max(0, i32(mContentSize.X) - i32(mVisibleSize.X));

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = pixels / (float)scrollableSize;

		mHorizontalScrollBar->Scroll(-offset);
	}
}

void GUIScrollArea::ScrollUpPercent(float percent)
{
	if(mVerticalScrollBar != nullptr)
		mVerticalScrollBar->Scroll(percent);
}

void GUIScrollArea::ScrollDownPercent(float percent)
{
	if(mVerticalScrollBar != nullptr)
		mVerticalScrollBar->Scroll(-percent);
}

void GUIScrollArea::ScrollLeftPercent(float percent)
{
	if(mHorizontalScrollBar != nullptr)
		mHorizontalScrollBar->Scroll(percent);
}

void GUIScrollArea::ScrollRightPercent(float percent)
{
	if(mHorizontalScrollBar != nullptr)
		mHorizontalScrollBar->Scroll(-percent);
}

bool GUIScrollArea::DoOnMouseEvent(const GUIMouseEvent& ev)
{
	if(ev.GetType() == GUIMouseEventType::MouseWheelScroll)
	{
		// Mouse wheel only scrolls on the Y axis
		if(mVerticalScrollBar != nullptr)
		{
			u32 scrollableHeight = (u32)std::max(0, i32(mContentSize.Y) - i32(mVisibleSize.Y));
			float additionalScroll = (float)kWheelScrollAmount / scrollableHeight;

			mVerticalScrollBar->Scroll(additionalScroll * ev.GetWheelScrollAmount());
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
