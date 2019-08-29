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
	class BS_EXPORT GUIScrollArea : public GUIElementContainer
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/**
		 * Creates a new empty scroll area.
		 *
		 * @param[in]	vertBarType		Vertical scrollbar options.
		 * @param[in]	horzBarType		Horizontal scrollbar options.
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* create(ScrollBarType vertBarType, ScrollBarType horzBarType,
			const String& scrollBarStyle = StringUtil::BLANK, const String& scrollAreaStyle = StringUtil::BLANK);

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
		static GUIScrollArea* create(ScrollBarType vertBarType, ScrollBarType horzBarType,
			const GUIOptions& options, const String& scrollBarStyle = StringUtil::BLANK,
			const String& scrollAreaStyle = StringUtil::BLANK);

		/**
		 * Creates a new empty scroll area. Scroll bars will be show if needed and hidden otherwise.
		 *
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* create(const String& scrollBarStyle = StringUtil::BLANK,
			const String& scrollAreaStyle = StringUtil::BLANK);

		/**
		 * Creates a new empty scroll area. Scroll bars will be show if needed and hidden otherwise.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	scrollBarStyle	Style used by the scroll bars.
		 * @param[in]	scrollAreaStyle	Style used by the scroll content area.
		 */
		static GUIScrollArea* create(const GUIOptions& options, const String& scrollBarStyle = StringUtil::BLANK,
			const String& scrollAreaStyle = StringUtil::BLANK);

		/**	Returns the scroll area layout that you may use to add elements inside the scroll area. */
		GUILayout& getLayout() const { return *mContentLayout; }

		/**	Scrolls the area up by specified amount of pixels, if possible. */
		void scrollUpPx(UINT32 pixels);

		/**	Scrolls the area down by specified amount of pixels, if possible. */
		void scrollDownPx(UINT32 pixels);

		/**	Scrolls the area left by specified amount of pixels, if possible. */
		void scrollLeftPx(UINT32 pixels);

		/**	Scrolls the area right by specified amount of pixels, if possible. */
		void scrollRightPx(UINT32 pixels);

		/**	Scrolls the area up by specified percentage (ranging [0, 1]), if possible. */
		void scrollUpPct(float percent);

		/**	Scrolls the area down by specified percentage (ranging [0, 1]), if possible. */
		void scrollDownPct(float percent);

		/**	Scrolls the area left by specified percentage (ranging [0, 1]), if possible. */
		void scrollLeftPct(float percent);

		/**	Scrolls the area right by specified percentage (ranging [0, 1]), if possible. */
		void scrollRightPct(float percent);

		/**
		 * Scrolls the contents to the specified position (0 meaning top-most part of the content is visible, and 1 meaning
		 * bottom-most part is visible).
		 */
		void scrollToVertical(float pct);

		/**
		 * Scrolls the contents to the specified position (0 meaning left-most part of the content is visible, and 1 meaning
		 * right-most part is visible)
		 */
		void scrollToHorizontal(float pct);

		/**
		 * Returns how much is the scroll area scrolled in the vertical direction. Returned value represents percentage
		 * where 0 means no scrolling is happening, and 1 means area is fully scrolled to the bottom.
		 */
		float getVerticalScroll() const;

		/**
		 * Returns how much is the scroll area scrolled in the horizontal direction. Returned value represents percentage
		 * where 0 means no scrolling is happening, and 1 means area is fully scrolled to the right.
		 */
		float getHorizontalScroll() const;

		/**
		 * Returns the bounds of the scroll area not including the scroll bars (meaning only the portion that contains the
		 * contents).
		 */
		Rect2I getContentBounds();

		/**
		 * Number of pixels the scroll bar will occupy when active. This is width for vertical scrollbar, and height for
		 * horizontal scrollbar.
		 */
		static const UINT32 ScrollBarWidth;

		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementContainer::_getElementType */
		ElementType _getElementType() const override { return ElementType::ScrollArea; }

		/** @} */
	protected:
		~GUIScrollArea() = default;

		/** @copydoc GUIElementContainer::_getLayoutSizeRange */
		LayoutSizeRange _getLayoutSizeRange() const override;

		/** @copydoc GUIElementContainer::updateClippedBounds */
		void updateClippedBounds() override;

		/** @copydoc GUIElementBase::_calculateLayoutSizeRange */
		LayoutSizeRange _calculateLayoutSizeRange() const override;

		/** @copydoc GUIElementBase::_updateOptimalLayoutSizes */
		void _updateOptimalLayoutSizes() override;

		/** @copydoc GUIElementContainer::_getOptimalSize */
		Vector2I _getOptimalSize() const override;
	private:
		GUIScrollArea(ScrollBarType vertBarType, ScrollBarType horzBarType,
			const String& scrollBarStyle, const String& scrollAreaStyle, const GUIDimensions& dimensions);

		/** @copydoc GUIElementContainer::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/**
		 * Called when the vertical scrollbar moves.
		 *
		 * @param[in]	pct	Scrollbar position ranging [0, 1].
		 */
		void vertScrollUpdate(float pct);

		/**
		 * Called when the horizontal scrollbar moves.
		 *
		 * @param[in]	pct	Scrollbar position ranging [0, 1].
		 */
		void horzScrollUpdate(float pct);

		/** @copydoc	GUIElementContainer::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc	GUIElementContainer::_getElementAreas */
		void _getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
			const Vector<LayoutSizeRange>& sizeRanges, const LayoutSizeRange& mySizeRange) const override;

		/**
		 * @copydoc	GUIElementContainer::_getElementAreas
		 *
		 * @note	Also calculates some scroll area specific values.
		 */
		void _getElementAreas(const Rect2I& layoutArea, Rect2I* elementAreas, UINT32 numElements,
			const Vector<LayoutSizeRange>& sizeRanges, Vector2I& visibleSize, Vector2I& contentSize) const;

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

		static const UINT32 MinHandleSize;
		static const UINT32 WheelScrollAmount;
	};

	/** @} */
}
