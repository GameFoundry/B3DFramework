//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#include "GUI/BsGUIScrollArea.h"

#include "BsGUILayoutX.h"
#include "BsGUIPanel.h"
#include "GUI/BsGUISizeConstraints.h"
#include "GUI/BsGUILayoutY.h"
#include "GUI/BsGUIVerticalScrollBar.h"
#include "GUI/BsGUIHorizontalScrollBar.h"
#include "GUI/BsGUIMouseEvent.h"
#include "GUI/BsGUIUtility.h"

using namespace std::placeholders;

using namespace bs;

/** @cond RTTI */
/** @addtogroup RTTI-Impl-Engine
 *  @{
 */

namespace bs
{
	class B3D_EXPORT GUIScrollAreaRTTI : public TRTTIType<GUIScrollArea, GUIElementContainer, GUIScrollAreaRTTI>
	{
	public:
		const String& GetRttiName()
		{
			static String name = "GUIScrollArea";
			return name;
		}

		u32 GetRttiId() const override { return TID_GUIScrollArea; }

		SPtr<IReflectable> NewRttiObject() { return nullptr; }
	};
} // namespace bs

// TODO
// [x] Add a GUIContent structure for initializing and remove the explicit create methods
// [x] Make sure its script exported via codegen
// [ ] Port GUILayout and derived types to use codegen
// [ ] Add an option to perform scroll via drag
//  - Ideally, just by dragging on an empty area (perhaps an underlay element that catches the drag event?, or add some lower level funcionality to handle this)
// [ ] Add an option to perform zoom
// [x] Need a way to specify the underlying layout element for the scroll area (GUILayoutY, GUIPanel, or perhaps even GUILayoutX)

const GUILogicalUnit GUIScrollArea::kScrollBarWidth = 16;
const u32 GUIScrollArea::kWheelScrollAmount = 50;

GUIScrollArea::GUIScrollArea(PrivatelyConstruct, const GUIScrollAreaContent& content, const String& styleClass, const GUISizeConstraints& sizeConstraints)
	: GUIElementContainer(sizeConstraints, styleClass), mContent(content)
{
	switch(content.LayoutType)
	{
	default:
	case ScrollAreaLayoutType::Vertical:
		mContentLayout = GUILayoutY::Create();
		break;
	case ScrollAreaLayoutType::Horizontal:
		mContentLayout = GUILayoutX::Create();
		break;
	case ScrollAreaLayoutType::Panel:
		mContentLayout = GUIPanel::Create();
		break;
	}
	RegisterChildElement(mContentLayout);

	mHorizontalScrollBar = GUIHorizontalScrollBar::Create();
	mVerticalScrollBar = GUIVerticalScrollBar::Create();

	RegisterChildElement(mHorizontalScrollBar);
	RegisterChildElement(mVerticalScrollBar);

	mHorizontalScrollBar->OnScrollOrResize.Connect([this](float scrollHandlePosition, float) { DoOnHorizontalScrollUpdate(scrollHandlePosition); });
	mVerticalScrollBar->OnScrollOrResize.Connect([this](float scrollHandlePosition, float) { DoOnVerticalScrollUpdate(scrollHandlePosition); });
}

GUILogicalSize GUIScrollArea::CalculateUnconstrainedOptimalSize() const
{
	// TODO - For layouts the function call below actually returns constrained size, despite the name
	GUILogicalSize optimalSize = mContentLayout->CalculateUnconstrainedOptimalSize();

	if(mContent.VerticalScrollBarType != ScrollBarType::NeverShow)
		optimalSize.Width += kScrollBarWidth;

	if(mContent.HorizontalScrollBarType != ScrollBarType::NeverShow)
		optimalSize.Height += kScrollBarWidth;

	// Provide 10x10 in case underlying layout is empty because
	// 0 doesn't work well with the layout system
	optimalSize.Width = Math::Max(optimalSize.Width, 10);
	optimalSize.Height = Math::Max(optimalSize.Height, 10);

	return optimalSize;
}

GUIConstrainedSize GUIScrollArea::CalculateConstrainedSize() const
{
	if(mContentLayout->IsActive())
		return mSizeConstraints.CalculateConstrainedSize(CalculateUnconstrainedOptimalSize());

	return mSizeConstraints.CalculateConstrainedSize(GUILogicalSize(BsZero));
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

void GUIScrollArea::CalculateRelativeElementAreas(const GUILogicalSize& scrollAreaSize,  GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, GUILogicalSize& outVisibleSize) const
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
	GUILogicalUnit optimalContentWidth = scrollAreaSize.Width;
	if(mContent.HorizontalScrollBarType != ScrollBarType::NeverShow)
		optimalContentWidth = sizeRanges[layoutIdx].Optimal.Width;

	GUILogicalUnit optimalContentHeight = scrollAreaSize.Height;
	if(mContent.VerticalScrollBarType != ScrollBarType::NeverShow)
		optimalContentHeight = sizeRanges[layoutIdx].Optimal.Height;

	GUILogicalUnit layoutWidth = Math::Max(optimalContentWidth, scrollAreaSize.Width);
	GUILogicalUnit layoutHeight = Math::Max(optimalContentHeight, scrollAreaSize.Height);

	outVisibleSize = scrollAreaSize;

	const bool hasHorizontalScrollbar = mContent.HorizontalScrollBarType != ScrollBarType::NeverShow;
	const bool hasVerticalScrollbar = mContent.VerticalScrollBarType != ScrollBarType::NeverShow;
	if(hasHorizontalScrollbar)
	{
		// Make room for scrollbar
		outVisibleSize.Height = Math::Max(scrollAreaSize.Height - kScrollBarWidth, 0);
		optimalContentHeight = Math::Max(optimalContentHeight - kScrollBarWidth, 0);

		if(sizeRanges[layoutIdx].Minimum.Height > 0)
			optimalContentHeight = Math::Max(sizeRanges[layoutIdx].Minimum.Height, optimalContentHeight);

		layoutHeight = Math::Max(optimalContentHeight, outVisibleSize.Height); // Never go below optimal size
	}

	if(hasVerticalScrollbar)
	{
		// Make room for scrollbar
		outVisibleSize.Width = Math::Max(scrollAreaSize.Width - kScrollBarWidth, 0);
		optimalContentWidth = Math::Max(optimalContentWidth - kScrollBarWidth, 0);

		if(sizeRanges[layoutIdx].Minimum.Width > 0)
			optimalContentWidth = Math::Max(sizeRanges[layoutIdx].Minimum.Width, optimalContentWidth);

		layoutWidth = Math::Max(optimalContentWidth, outVisibleSize.Width); // Never go below optimal size
	}

	outElementSizes[layoutIdx] = GUILogicalSize(layoutWidth, layoutHeight);
	outElementPositions[layoutIdx] = GUILogicalPoint(0, 0);

	// Calculate vertical scrollbar bounds
	if(hasVerticalScrollbar)
	{
		GUILogicalUnit scrollBarOffset = Math::Max(scrollAreaSize.Width - kScrollBarWidth, 0);
		GUILogicalUnit scrollBarHeight = scrollAreaSize.Height;
		if(hasHorizontalScrollbar)
			scrollBarHeight = Math::Max(scrollBarHeight - kScrollBarWidth, 0);

		outElementSizes[vertScrollIdx] = GUILogicalSize(kScrollBarWidth, scrollBarHeight);
		outElementPositions[vertScrollIdx] = GUILogicalPoint( scrollBarOffset, 0);
	}
	else
	{
		outElementSizes[vertScrollIdx] = GUILogicalSize(0, 0);
		outElementPositions[vertScrollIdx] = GUILogicalPoint(layoutWidth, 0);
	}

	// Calculate horizontal scrollbar bounds
	if(hasHorizontalScrollbar)
	{
		GUILogicalUnit scrollBarOffset = Math::Max(scrollAreaSize.Height - kScrollBarWidth, 0);
		GUILogicalUnit scrollBarWidth = scrollAreaSize.Width;
		if(hasVerticalScrollbar)
			scrollBarWidth = Math::Max(scrollBarWidth - kScrollBarWidth, 0);

		outElementSizes[horzScrollIdx] = GUILogicalSize(scrollBarWidth, kScrollBarWidth);
		outElementPositions[horzScrollIdx] = GUILogicalPoint(0, scrollBarOffset);
	}
	else
	{
		outElementSizes[horzScrollIdx] = GUILogicalSize(0, 0);
		outElementPositions[horzScrollIdx] = GUILogicalPoint(0, layoutHeight);
	}
}

void GUIScrollArea::UpdateLayoutForChildren()
{
	const u32 elementCount = (u32)mChildren.size();
	GUILogicalPoint* elementPositions = nullptr;
	GUILogicalSize* elementSizes = nullptr;

	if(elementCount > 0)
	{
		elementPositions = B3DStackNew<GUILogicalPoint>(elementCount);
		elementSizes = B3DStackNew<GUILogicalSize>(elementCount);
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

	const GUILogicalSize scrollAreaSize = mLayoutData.Size;
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

	mContentSize = elementSizes[layoutIdx];

	const bool showVerticalScrollBar = mContent.VerticalScrollBarType != ScrollBarType::NeverShow && mContentSize.Height > mVisibleSize.Height;
	const bool showHorizontalScrollBar = mContent.HorizontalScrollBarType != ScrollBarType::NeverShow && mContentSize.Width > mVisibleSize.Width;

	// Vertical scrollbar
	{
		GUILayoutData layoutData = mLayoutData;
		layoutData.RelativePosition = elementPositions[vertScrollIdx];

		if(showVerticalScrollBar)
			layoutData.Size = elementSizes[vertScrollIdx];
		else
			layoutData.Size = GUILogicalSize(0, 0);

		mVerticalScrollBar->SetLayoutData(layoutData);
		mVerticalScrollBar->UpdateLayoutForChildren();

		// Set new handle size and update position to match the new size
		GUILogicalUnit scrollableHeight = Math::Max(mContentSize.Height - layoutData.Size.Height, 0);
		float newScrollPct = 0.0f;

		if(scrollableHeight > 0)
			newScrollPct = mVerticalOffset / (float)scrollableHeight;

		mVerticalScrollBar->SetHandleSizeInternal((float)layoutData.Size.Height / (float)mContentSize.Height);
		mVerticalScrollBar->SetScrollPosInternal(newScrollPct);
	}

	// Horizontal scrollbar
	{
		GUILayoutData layoutData = mLayoutData;
		layoutData.RelativePosition = elementPositions[horzScrollIdx];

		if(showHorizontalScrollBar)
			layoutData.Size = elementSizes[horzScrollIdx];
		else
			layoutData.Size = GUILogicalSize(0, 0);

		mHorizontalScrollBar->SetLayoutData(layoutData);
		mHorizontalScrollBar->UpdateLayoutForChildren();

		// Set new handle size and update position to match the new size
		GUILogicalUnit scrollableWidth = Math::Max(mContentSize.Width - layoutData.Size.Width, 0);
		float newScrollPct = 0.0f;

		if(scrollableWidth > 0)
			newScrollPct = mHorizontalOffset / (float)scrollableWidth;

		mHorizontalScrollBar->SetHandleSizeInternal((float)layoutData.Size.Width / (float)mContentSize.Width);
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
	GUILogicalUnit scrollableHeight = Math::Max(mContentSize.Height - mVisibleSize.Height, 0);
	if(mRecalculateVerticalOffset)
	{
		mVerticalOffset = (float)scrollableHeight * Math::Clamp01(mVerticalScrollBar->GetScrollHandlePosition());
		mRecalculateVerticalOffset = false;
	}

	GUILogicalUnit scrollableWidth = Math::Max(mContentSize.Width - mVisibleSize.Width, 0);
	if(mRecalculateHorizontalOffset)
	{
		mHorizontalOffset = (float)scrollableWidth * Math::Clamp01(mHorizontalScrollBar->GetScrollHandlePosition());
		mRecalculateHorizontalOffset = false;
	}

	// Reset offset in case layout size changed so everything fits
	mVerticalOffset = Math::Clamp(mVerticalOffset, 0.0f, (float)scrollableHeight);
	mHorizontalOffset = Math::Clamp(mHorizontalOffset, 0.0f, (float)scrollableWidth);

	if(mContentLayout->IsActive())
	{
		const GUIPhysicalPointF contentOrigin(mIntermediateAbsolutePosition.X - mHorizontalOffset * mAbsoluteScale, mIntermediateAbsolutePosition.Y - mVerticalOffset * mAbsoluteScale);
		GUIPhysicalAreaF contentVisibleAreaSize(mIntermediateAbsolutePosition, mVisibleSize.To<GUIPhysicalUnitF>() * mAbsoluteScale);
		contentVisibleAreaSize.Clip(mIntermediateAbsoluteClippedArea);

		mContentLayout->UpdateAbsoluteCoordinates(contentOrigin, mAbsoluteScale, contentVisibleAreaSize);
	}

	mHorizontalScrollBar->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
	mVerticalScrollBar->UpdateAbsoluteCoordinates(mIntermediateAbsolutePosition, mAbsoluteScale, mIntermediateAbsoluteClippedArea);
}

void GUIScrollArea::DoOnVerticalScrollUpdate(float scrollPos)
{
	GUILogicalUnit scrollableHeight = Math::Max(mContentSize.Height - mVisibleSize.Height, 0);
	mVerticalOffset = (float)scrollableHeight * Math::Clamp01(scrollPos);

	MarkAbsoluteCoordinatesAsDirty();
}

void GUIScrollArea::DoOnHorizontalScrollUpdate(float scrollPos)
{
	GUILogicalUnit scrollableWidth = Math::Max(mContentSize.Width - mVisibleSize.Width, 0);
	mHorizontalOffset = (float)scrollableWidth * Math::Clamp01(scrollPos);

	MarkAbsoluteCoordinatesAsDirty();
}

void GUIScrollArea::ScrollToVertical(float pct)
{
	mVerticalScrollBar->SetScrollPosInternal(pct);
	mRecalculateVerticalOffset = true;

	MarkAbsoluteCoordinatesAsDirty();
}

void GUIScrollArea::ScrollToHorizontal(float pct)
{
	mHorizontalScrollBar->SetScrollPosInternal(pct);
	mRecalculateHorizontalOffset = true;

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

GUIPhysicalArea GUIScrollArea::GetContentBounds()
{
	GUIPhysicalArea bounds = CalculateAbsoluteBoundsRelativeTo();
	const GUIPhysicalUnit physicalScrollBarWidth = GUIUtility::LogicalToPhysical(kScrollBarWidth, GetAbsoluteScale());

	if(mHorizontalScrollBar)
		bounds.Height -= physicalScrollBarWidth;

	if(mVerticalScrollBar)
		bounds.Width -= physicalScrollBarWidth;

	return bounds;
}

void GUIScrollArea::SetEnableCulling(bool enable)
{
	if(mContentLayout != nullptr)
		mContentLayout->SetEnableCulling(enable);
}

void GUIScrollArea::ScrollUp(GUIPhysicalUnit pixels)
{
	if(mVerticalScrollBar != nullptr)
	{
		const GUILogicalUnit logicalPixels = GUIUtility::PhysicalToLogical(pixels, GetAbsoluteScale());
		const GUILogicalUnit scrollableSize = Math::Max(mContentSize.Height - mVisibleSize.Height, 0);

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = (float)logicalPixels / (float)scrollableSize;

		mVerticalScrollBar->Scroll(offset);
	}
}

void GUIScrollArea::ScrollDown(GUIPhysicalUnit pixels)
{
	if(mVerticalScrollBar != nullptr)
	{
		const GUILogicalUnit logicalPixels = GUIUtility::PhysicalToLogical(pixels, GetAbsoluteScale());
		const GUILogicalUnit scrollableSize = Math::Max(mContentSize.Height - mVisibleSize.Height, 0);

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = (float)logicalPixels / (float)scrollableSize;

		mVerticalScrollBar->Scroll(-offset);
	}
}

void GUIScrollArea::ScrollLeft(GUIPhysicalUnit pixels)
{
	if(mHorizontalScrollBar != nullptr)
	{
		const GUILogicalUnit logicalPixels = GUIUtility::PhysicalToLogical(pixels, GetAbsoluteScale());
		const GUILogicalUnit scrollableSize = Math::Max(mContentSize.Width - mVisibleSize.Width, 0);

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = (float)logicalPixels / (float)scrollableSize;

		mHorizontalScrollBar->Scroll(offset);
	}
}

void GUIScrollArea::ScrollRight(GUIPhysicalUnit pixels)
{
	if(mHorizontalScrollBar != nullptr)
	{
		const GUILogicalUnit logicalPixels = GUIUtility::PhysicalToLogical(pixels, GetAbsoluteScale());
		const GUILogicalUnit scrollableSize = Math::Max(mContentSize.Width - mVisibleSize.Width, 0);

		float offset = 0.0f;
		if(scrollableSize > 0)
			offset = (float)logicalPixels / (float)scrollableSize;

		mHorizontalScrollBar->Scroll(-offset);
	}
}

void GUIScrollArea::ScrollUp(float percent)
{
	if(mVerticalScrollBar != nullptr)
		mVerticalScrollBar->Scroll(percent);
}

void GUIScrollArea::ScrollDown(float percent)
{
	if(mVerticalScrollBar != nullptr)
		mVerticalScrollBar->Scroll(-percent);
}

void GUIScrollArea::ScrollLeft(float percent)
{
	if(mHorizontalScrollBar != nullptr)
		mHorizontalScrollBar->Scroll(percent);
}

void GUIScrollArea::ScrollRight(float percent)
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
			GUILogicalUnit scrollableHeight = Math::Max(mContentSize.Height - mVisibleSize.Height, 0);
			float additionalScroll = (float)kWheelScrollAmount / (float)scrollableHeight;

			mVerticalScrollBar->Scroll(additionalScroll * ev.GetWheelScrollAmount());
			return true;
		}
	}

	return false;
}

const String& GUIScrollArea::GetGuiTypeName()
{
	static String typeName = "ScrollArea";
	return typeName;
}

RTTIType* GUIScrollArea::GetRttiStatic()
{
	return GUIScrollAreaRTTI::Instance();
}

RTTIType* GUIScrollArea::GetRtti() const
{
	return GetRttiStatic();
}
