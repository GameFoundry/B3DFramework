//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementContainer.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Scroll bar options for a GUI scroll area. */
	enum class ScrollBarType
	{
		ShowIfDoesntFit,
		AlwaysShow,
		NeverShow
	};

	/**	A GUI element container with support for vertical & horizontal scrolling. */
	class B3D_EXPORT GUIScrollArea : public GUIElementContainer
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new empty scroll area.
		 *
		 * @param[in]	vertBarType		Vertical scrollbar options.
		 * @param[in]	horzBarType		Horizontal scrollbar options.
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* Create(ScrollBarType vertBarType, ScrollBarType horzBarType, const String& scrollBarStyle = StringUtil::kBlank, const String& scrollAreaStyle = StringUtil::kBlank);

		/**
		 * Creates a new empty scroll area.
		 *
		 * @param[in]	vertBarType		Vertical scrollbar options.
		 * @param[in]	horzBarType		Horizontal scrollbar options.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* Create(ScrollBarType vertBarType, ScrollBarType horzBarType, const GUIOptions& options, const String& scrollBarStyle = StringUtil::kBlank, const String& scrollAreaStyle = StringUtil::kBlank);

		/**
		 * Creates a new empty scroll area. Scroll bars will be show if needed and hidden otherwise.
		 *
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* Create(const String& scrollBarStyle = StringUtil::kBlank, const String& scrollAreaStyle = StringUtil::kBlank);

		/**
		 * Creates a new empty scroll area. Scroll bars will be show if needed and hidden otherwise.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* Create(const GUIOptions& options, const String& scrollBarStyle = StringUtil::kBlank, const String& scrollAreaStyle = StringUtil::kBlank);

		/**	Returns the scroll area layout that you may use to add elements inside the scroll area. */
		GUILayoutY& GetLayout() const { return *mContentLayout; }

		/**	Scrolls the area up by specified amount of pixels, if possible. */
		void ScrollUpPixels(GUIPhysicalUnit pixels);

		/**	Scrolls the area down by specified amount of pixels, if possible. */
		void ScrollDownPixels(GUIPhysicalUnit pixels);

		/**	Scrolls the area left by specified amount of pixels, if possible. */
		void ScrollLeftPixels(GUIPhysicalUnit pixels);

		/**	Scrolls the area right by specified amount of pixels, if possible. */
		void ScrollRightPixels(GUIPhysicalUnit pixels);

		/**	Scrolls the area up by specified percentage (ranging [0, 1]), if possible. */
		void ScrollUpPercent(float percent);

		/**	Scrolls the area down by specified percentage (ranging [0, 1]), if possible. */
		void ScrollDownPercent(float percent);

		/**	Scrolls the area left by specified percentage (ranging [0, 1]), if possible. */
		void ScrollLeftPercent(float percent);

		/**	Scrolls the area right by specified percentage (ranging [0, 1]), if possible. */
		void ScrollRightPercent(float percent);

		/**
		 * Scrolls the contents to the specified position (0 meaning top-most part of the content is visible, and 1 meaning
		 * bottom-most part is visible).
		 */
		void ScrollToVertical(float pct);

		/**
		 * Scrolls the contents to the specified position (0 meaning left-most part of the content is visible, and 1 meaning
		 * right-most part is visible)
		 */
		void ScrollToHorizontal(float pct);

		/**
		 * Returns how much is the scroll area scrolled in the vertical direction. Returned value represents percentage
		 * where 0 means no scrolling is happening, and 1 means area is fully scrolled to the bottom.
		 */
		float GetVerticalScroll() const;

		/**
		 * Returns how much is the scroll area scrolled in the horizontal direction. Returned value represents percentage
		 * where 0 means no scrolling is happening, and 1 means area is fully scrolled to the right.
		 */
		float GetHorizontalScroll() const;

		/**
		 * Returns the bounds of the scroll area not including the scroll bars (meaning only the portion that contains the
		 * contents).
		 */
		GUIPhysicalArea GetContentBounds();

		/**
		 * Enables/disables culling of child elements. If culling is enabled all child elements that are fully outside of the parent visible bounds will be marked as culled.
		 * Culled elements will never have their contents or mesh updated, their absolute coordinate will not be updated and they wont be drawn
		 * This is useful for layouts with a large amount of children, but comes with an overhead so it is disabled by default. Note this has no impact on layout update,
		 * which may still be expensive with many elements.
		 */
		void SetEnableCulling(bool enable);

		/**
		 * Number of pixels the scroll bar will occupy when active. This is width for vertical scrollbar, and height for
		 * horizontal scrollbar.
		 */
		static const GUILogicalUnit kScrollBarWidth;

		/** @name Internal
		 *  @{
		 */

		ElementType GetElementType() const override { return ElementType::ScrollArea; }

		/** @} */
	protected:
		~GUIScrollArea() = default;

		GUIConstrainedSize GetConstrainedSize() const override;
		GUIConstrainedSize CalculateConstrainedSize() const override;
		void UpdateOptimalLayoutSizes() override;
		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

	private:
		GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType, const String& scrollBarStyle, const String& scrollAreaStyle, const GUISizeConstraints& dimensions);

		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;

		/**
		 * Called when the vertical scrollbar moves.
		 *
		 * @param[in]	pct	Scrollbar position ranging [0, 1].
		 */
		void VertScrollUpdate(float pct);

		/**
		 * Called when the horizontal scrollbar moves.
		 *
		 * @param[in]	pct	Scrollbar position ranging [0, 1].
		 */
		void HorzScrollUpdate(float pct);

		void UpdateLayoutForChildren() override;
		void UpdateAbsoluteCoordinatesForChildren() override;

		/** Calculates the position and size of the scroll area child layout and the scroll bars. */
		void CalculateRelativeElementAreas(const GUILogicalSize& scrollAreaSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSize>& sizeRanges, GUILogicalSize& outVisibleSize) const;

		ScrollBarType mVerticalScrollBarType;
		ScrollBarType mHorizontalScrollBarType;
		String mScrollBarStyle;

		GUILayoutY* mContentLayout;
		GUIVerticalScrollBar* mVerticalScrollBar;
		GUIHorizontalScrollBar* mHorizontalScrollBar;

		float mVertOffset;
		float mHorzOffset;
		bool mRecalculateVertOffset;
		bool mRecalculateHorzOffset;

		GUILogicalSize mVisibleSize{BsZero};
		GUILogicalSize mContentSize{BsZero};

		Vector<GUIConstrainedSize> mChildSizeRanges;
		GUIConstrainedSize mSizeRange;

		static const u32 kMinHandleSize;
		static const u32 kWheelScrollAmount;
	};

	/** @} */
} // namespace bs
