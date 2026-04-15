//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DGUISpriteHelper.h"
#include "B3DPrerequisites.h"
#include "GUI/B3DGUIInteractable.h"
#include "Utility/B3DEvent.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** GUI element representing an element with a draggable handle of a variable size. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIScrollBar : public GUIInteractable
	{
	public:
		/**	Position of the scroll handle in percent (ranging [0, 1]). */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(ScrollHandlePosition))
		void SetScrollHandlePosition(float pct);

		/**	@copydoc SetScrollHandlePosition */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ScrollHandlePosition))
		float GetScrollHandlePosition() const;

		/** Size of the scroll handle in percent (ranging [0, 1]) of the total scroll bar area. */
		B3D_SCRIPT_EXPORT(Property(Setter), ExportName(ScrollHandleSize))
		void SetScrollHandleSize(float pct);

		/**	@copydoc SetScrollHandleSize */
		B3D_SCRIPT_EXPORT(Property(Getter), ExportName(ScrollHandleSize))
		float GetScrollHandleSize() const;

		/**
		 * Moves the handle by some amount. Amount is specified in the percentage of the entire scrollable area. Values out
		 * of range will be clamped.
		 */
		void Scroll(float amount);

		/**	Returns the maximum scrollable size the handle can move within (for example scroll bar length). */
		GUIPhysicalUnit GetScrollableSize() const;

		void SetTint(const Color& color) override;
		bool IsInInteractionBounds(const GUIPhysicalPoint& position) const override { return false; } // Non-interactable

		/**
		 * Triggered whenever the scrollbar handle is moved or resized. Values provided are the handle position and size
		 * in percent (ranging [0, 1]).
		 */
		B3D_SCRIPT_EXPORT()
		Event<void(float scrollHandlePositionInPercent, float scrollHandleSizeInPercent)> OnScrollOrResize;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		const char* GetStyleSheetElement() const override { return "scrollbar"; }

		/**
		 * Sets the size of the handle in percent (ranging [0, 1]) of the total scroll bar area.
		 *
		 * @note	Does not trigger layout update.
		 */
		void SetHandleSizeInternal(float pct);

		/**
		 * Sets the position of the scroll handle in percent (ranging [0, 1]).
		 *
		 * @note	Does not trigger layout update.
		 */
		void SetScrollPosInternal(float pct);

		GUILogicalSize CalculateUnconstrainedOptimalSize() const override;
		void UpdateLayoutForChildren() override;

		/** @} */
	protected:
		/**
		 * Constructs a new scrollbar.
		 *
		 * @param	horizontal	If true the scroll bar will have a horizontal moving handle, otherwise it will be a
		 *						vertical one.
		 * @param	resizable	If true the scrollbar will have additional handles that allow the scroll handle to be
		 *						resized. This allows you to adjust the size of the visible scroll area.
		 * @param	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *						GUIWidget the element is used on. If not specified default style is used.
		 * @param	dimensions	Determines valid dimensions (size) of the element.
		 */
		GUIScrollBar(bool horizontal, bool resizable, const String& styleName, const GUISizeConstraints& dimensions);
		virtual ~GUIScrollBar();

		void UpdateRenderElements() override;
		u32 GetRenderElementDepthRange() const override;

		static constexpr const char* kHorizontalHandleStyleClass = "ScrollBarHorizontalHandle";
		static constexpr const char* kVerticalHandleStyleClass = "ScrollBarVerticalHandle";
		static constexpr const char* kResizableHorizontalHandleStyleClass = "ResizableScrollBarHorizontalHandle";
		static constexpr const char* kResizableVerticalHandleStyleClass = "ResizableScrollBarVerticalHandle";
		static constexpr const char* kScrollButtonStyleClass = "ScrollButton";

		/**
		 * Helper method that returns style name used by a specific scrollbar type. If override style is empty, default
		 * style for that type is returned.
		 */
		template <class T>
		static const String& GetStyleName(bool resizeable, const String& overrideStyle)
		{
			if(overrideStyle == StringUtility::kBlank)
				return T::GetGuiTypeName(resizeable);

			return overrideStyle;
		}

	private:
		/**
		 * Triggered whenever the scroll handle moves. Provided value represents the new position and size of the handle
		 * in percent (ranging [0, 1]).
		 */
		void HandleMoved(float handlePct, float sizePct);

		/**	Triggered when scroll up button is clicked. */
		void UpButtonClicked();

		/**	Triggered when scroll down button is clicked. */
		void DownButtonClicked();

		GUILayout* mLayout;
		GUIBackgroundSprite mBackgroundSprite;

		GUIButton* mUpBtn;
		GUIButton* mDownBtn;
		GUISliderHandle* mHandleBtn;
		bool mHorizontal;

		static const GUIPhysicalUnit kButtonScrollAmount;
	};

	/** @} */
} // namespace b3d
