//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/** GUI element representing an element with a draggable handle of a variable size. */
	class B3D_EXPORT GUIScrollBar : public GUIElement
	{
	public:
		/** Style type name for the horizontal scroll handle. */
		static const String& GetHScrollHandleType();

		/** Style type name for the vertical scroll handle. */
		static const String& GetVScrollHandleType();

		/**	Returns the position of the scroll handle in percent (ranging [0, 1]). */
		float GetScrollPos() const;

		/** Sets the position of the scroll handle in percent (ranging [0, 1]). */
		void SetScrollPos(float pct);

		/** Gets the size of the scroll handle in percent (ranging [0, 1]) of the total scroll bar area. */
		float GetHandleSize() const;

		/** Sets the size of the scroll handle in percent (ranging [0, 1]) of the total scroll bar area. */
		void SetHandleSize(float pct);

		/**
		 * Moves the handle by some amount. Amount is specified in the percentage of the entire scrollable area. Values out
		 * of range will be clamped.
		 */
		void Scroll(float amount);

		/**	Returns the maximum scrollable size the handle can move within (for example scroll bar length). */
		u32 GetScrollableSize() const;

		void SetTint(const Color& color) override;

		/**
		 * Triggered whenever the scrollbar handle is moved or resized. Values provided are the handle position and size
		 * in percent (ranging [0, 1]).
		 */
		Event<void(float posPct, float sizePct)> OnScrollOrResize;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

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

		Vector2I GetOptimalSize() const override;

		/** @} */
	protected:
		/**
		 * Constructs a new scrollbar.
		 *
		 * @param[in]	horizontal	If true the scroll bar will have a horizontal moving handle, otherwise it will be a
		 *							vertical one.
		 * @param[in]	resizable	If true the scrollbar will have additional handles that allow the scroll handle to be
		 *							resized. This allows you to adjust the size of the visible scroll area.
		 * @param[in]	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default style is used.
		 * @param[in]	dimensions	Determines valid dimensions (size) of the element.
		 */
		GUIScrollBar(bool horizontal, bool resizable, const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUIScrollBar();

		void FillBuffer(u8* vertices, u32* indices, u32 vertexOffset, u32 indexOffset, const Vector2I& offset, u32 maxNumVerts, u32 maxNumIndices, u32 renderElementIdx) const override;
		void UpdateRenderElements() override;
		void UpdateClippedBounds() override;
		u32 GetRenderElementDepthRange() const override;
		void StyleUpdated() override;

		/**
		 * Helper method that returns style name used by a specific scrollbar type. If override style is empty, default
		 * style for that type is returned.
		 */
		template <class T>
		static const String& GetStyleName(bool resizeable, const String& overrideStyle)
		{
			if(overrideStyle == StringUtil::kBlank)
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
		ImageSprite* mImageSprite;

		GUIButton* mUpBtn;
		GUIButton* mDownBtn;
		GUISliderHandle* mHandleBtn;
		bool mHorizontal;

		static const u32 kButtonScrollAmount;
	};

	/** @} */
} // namespace bs
