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
		GUILayout& GetLayout() const { return *mContentLayout; }

		/**	Scrolls the area up by specified amount of pixels, if possible. */
		void ScrollUpPx(u32 pixels);

		/**	Scrolls the area down by specified amount of pixels, if possible. */
		void ScrollDownPx(u32 pixels);

		/**	Scrolls the area left by specified amount of pixels, if possible. */
		void ScrollLeftPx(u32 pixels);

		/**	Scrolls the area right by specified amount of pixels, if possible. */
		void ScrollRightPx(u32 pixels);

		/**	Scrolls the area up by specified percentage (ranging [0, 1]), if possible. */
		void ScrollUpPct(float percent);

		/**	Scrolls the area down by specified percentage (ranging [0, 1]), if possible. */
		void ScrollDownPct(float percent);

		/**	Scrolls the area left by specified percentage (ranging [0, 1]), if possible. */
		void ScrollLeftPct(float percent);

		/**	Scrolls the area right by specified percentage (ranging [0, 1]), if possible. */
		void ScrollRightPct(float percent);

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
		Rect2I GetContentBounds();

		/**
		 * Number of pixels the scroll bar will occupy when active. This is width for vertical scrollbar, and height for
		 * horizontal scrollbar.
		 */
		static const u32 kScrollBarWidth;

		/** @name Internal
		 *  @{
		 */

		ElementType GetElementTypeInternal() const override { return ElementType::ScrollArea; }

		/** @} */
	protected:
		~GUIScrollArea() = default;

		LayoutSizeRange GetLayoutSizeRange() const override;
		void UpdateClippedBounds() override;
		LayoutSizeRange CalculateLayoutSizeRange() const override;
		void UpdateOptimalLayoutSizes() override;
		Vector2I GetOptimalSize() const override;

	private:
		GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType, const String& scrollBarStyle, const String& scrollAreaStyle, const GUIDimensions& dimensions);

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

		void UpdateLayoutRecursive(const GUILayoutData& data) override;

		void GetElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/**
		 * @copydoc	GUIElementContainer::GetElementAreasInternal:
		 *
		 * @note	Also calculates some scroll area specific values.
		 */
		void GetElementAreasInternal(const Rect2I& layoutArea, Rect2I* elementAreas, u32 numElements, const Vector<LayoutSizeRange>& sizeRanges, Vector2I& visibleSize, Vector2I& contentSize) const;

		ScrollBarType mVertBarType;
		ScrollBarType mHorzBarType;
		String mScrollBarStyle;

		GUILayout* mContentLayout;
		GUIScrollBarVert* mVertScroll;
		GUIScrollBarHorz* mHorzScroll;

		float mVertOffset;
		float mHorzOffset;
		bool mRecalculateVertOffset;
		bool mRecalculateHorzOffset;

		Vector2I mVisibleSize;
		Vector2I mContentSize;

		Vector<LayoutSizeRange> mChildSizeRanges;
		LayoutSizeRange mSizeRange;

		static const u32 kMinHandleSize;
		static const u32 kWheelScrollAmount;
	};

	/** @} */
} // namespace bs
