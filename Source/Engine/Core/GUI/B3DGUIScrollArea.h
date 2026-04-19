//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUIConstructionMethods.h"
#include "B3DGUILayoutY.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIElementContainer.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Determines when and if to display a scroll bar on the scroll area. */
	enum class B3D_SCRIPT_EXPORT() ScrollBarType
	{
		ShowIfDoesntFit,
		AlwaysShow,
		NeverShow
	};

	/** Determines how are elements positioned in a scroll area. */
	enum class B3D_SCRIPT_EXPORT() ScrollAreaLayoutType
	{
		Vertical, /**< GUI elements will be placed below/above each other (e.g. as in GUILayoutY). This is the standard scroll area. */
		Horizontal, /**< GUI elements will be placed left/right to each other (e.g. as in GUILayoutX). */
		Panel, /**< GUI elements must be explicitly positioned (e.g. as in GUIPanel). */
	};

	/** Structure describing contents of a GUIScrollArea element. */
	struct B3D_SCRIPT_EXPORT(ExportAsStruct(true), DocumentationGroup(GUI)) GUIScrollAreaContent
	{
		GUIScrollAreaContent(ScrollBarType verticalScrollBarType = ScrollBarType::ShowIfDoesntFit, ScrollBarType horizontalScrollBarType = ScrollBarType::NeverShow, ScrollAreaLayoutType layoutType = ScrollAreaLayoutType::Vertical)
			: VerticalScrollBarType(verticalScrollBarType), HorizontalScrollBarType(horizontalScrollBarType), LayoutType(layoutType)
		{ }

		ScrollBarType VerticalScrollBarType;
		ScrollBarType HorizontalScrollBarType;
		ScrollAreaLayoutType LayoutType;
	};

	/**	A GUI element container with support for vertical & horizontal scrolling. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIScrollArea : public GUIElementContainer, public TGUIConstructionMethods<GUIScrollArea, GUIScrollAreaContent>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& GetGuiTypeName();

		~GUIScrollArea() = default;

		/**	Returns the scroll area layout that you may use to add elements inside the scroll area. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(Layout))
		GUILayout* GetLayout() const { return mContentLayout; }

		/**	Scrolls the area up by specified amount of pixels, if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollUp(GUIPhysicalUnit pixels);

		/**	Scrolls the area down by specified amount of pixels, if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollDown(GUIPhysicalUnit pixels);

		/**	Scrolls the area left by specified amount of pixels, if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollLeft(GUIPhysicalUnit pixels);

		/**	Scrolls the area right by specified amount of pixels, if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollRight(GUIPhysicalUnit pixels);

		/**	Scrolls the area up by specified percentage (ranging [0, 1]), if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollUp(float percent);

		/**	Scrolls the area down by specified percentage (ranging [0, 1]), if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollDown(float percent);

		/**	Scrolls the area left by specified percentage (ranging [0, 1]), if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollLeft(float percent);

		/**	Scrolls the area right by specified percentage (ranging [0, 1]), if possible. */
		B3D_SCRIPT_EXPORT()
		void ScrollRight(float percent);

		/**
		 * Scrolls the contents to the specified position (0 meaning top-most part of the content is visible, and 1 meaning
		 * bottom-most part is visible).
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(VerticalScroll))
		void ScrollToVertical(float pct);

		/**
		 * Scrolls the contents to the specified position (0 meaning left-most part of the content is visible, and 1 meaning
		 * right-most part is visible)
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(HorizontalScroll))
		void ScrollToHorizontal(float pct);

		/**
		 * Returns how much is the scroll area scrolled in the vertical direction. Returned value represents percentage
		 * where 0 means no scrolling is happening, and 1 means area is fully scrolled to the bottom.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(VerticalScroll))
		float GetVerticalScroll() const;

		/**
		 * Returns how much is the scroll area scrolled in the horizontal direction. Returned value represents percentage
		 * where 0 means no scrolling is happening, and 1 means area is fully scrolled to the right.
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(HorizontalScroll))
		float GetHorizontalScroll() const;

		/**
		 * Returns the bounds of the scroll area not including the scroll bars (meaning only the portion that contains the
		 * contents).
		 */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ContentBounds))
		GUIPhysicalArea GetContentBounds();

		/**
		 * Enables/disables culling of child elements. If culling is enabled all child elements that are fully outside of the parent visible bounds will be marked as culled.
		 * Culled elements will never have their contents or mesh updated, their absolute coordinate will not be updated and they wont be drawn
		 * This is useful for layouts with a large amount of children, but comes with an overhead so it is disabled by default. Note this has no impact on layout update,
		 * which may still be expensive with many elements.
		 */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(EnableCulling))
		void SetEnableCulling(bool enable);

		/** Returns the width or height of the scrollbar. */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ScrollBarSize))
		GUILogicalUnit GetScrollBarSize() const { return kScrollBarWidth; }

		/**
		 * Number of pixels the scroll bar will occupy when active. This is width for vertical scrollbar, and height for
		 * horizontal scrollbar.
		 */
		static const GUILogicalUnit kScrollBarWidth;

		/**
		 * @name Internal
		 * @{
		 */

		struct PrivatelyConstruct {};
		GUIScrollArea(PrivatelyConstruct, const GUIScrollAreaContent& content, const String& styleClass, const GUISizeConstraints& sizeConstraints);

		/** @} */

	protected:
		GUIConstrainedSizeRange GetConstrainedSizeRange() const override;
		GUIConstrainedSizeRange CalculateConstrainedSizeRange() const override;
		void UpdateOptimalLayoutSizes() override;
		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;

	private:
		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;

		/**
		 * Called when the vertical scrollbar moves.
		 *
		 * @param	scrollHandlePosition	Scrollbar position ranging [0, 1].
		 */
		void DoOnVerticalScrollUpdate(float scrollHandlePosition);

		/**
		 * Called when the horizontal scrollbar moves.
		 *
		 * @param	scrollHandlePosition	Scrollbar position ranging [0, 1].
		 */
		void DoOnHorizontalScrollUpdate(float scrollHandlePosition);

		void UpdateLayoutForChildren() override;
		void UpdateAbsoluteCoordinatesForChildren() override;

		/** Calculates the position and size of the scroll area child layout and the scroll bars. */
		void CalculateRelativeElementAreas(const GUILogicalSize& scrollAreaSize, GUILogicalPoint* outElementPositions, GUILogicalSize* outElementSizes, u32 elementCount, const Vector<GUIConstrainedSizeRange>& constrainedSizeRanges, GUILogicalSize& outVisibleSize) const;

		GUIScrollAreaContent mContent;

		GUILayout* mContentLayout = nullptr;
		GUIVerticalScrollBar* mVerticalScrollBar = nullptr;
		GUIHorizontalScrollBar* mHorizontalScrollBar = nullptr;

		float mVerticalOffset = 0.0f;
		float mHorizontalOffset = 0.0f;
		bool mRecalculateVerticalOffset = false;
		bool mRecalculateHorizontalOffset = false;
		bool mDragInProgress = false;
		GUIPhysicalPoint mDragStartPosition{kZeroTag};
		GUIPhysicalPoint mDragStartOffset{kZeroTag};

		GUILogicalSize mVisibleSize{kZeroTag};
		GUILogicalSize mContentSize{kZeroTag};

		Vector<GUIConstrainedSizeRange> mChildSizeRanges;
		GUIConstrainedSizeRange mSizeRange;

		static const u32 kMinHandleSize;
		static const u32 kWheelScrollAmount;

		/************************************************************************/
		/* 								RTTI		                     		*/
		/************************************************************************/
	public:
		friend class GUIScrollAreaRTTI;
		static RTTIType* GetRttiStatic();
		RTTIType* GetRtti() const override;
	};

	/** @} */
} // namespace b3d
