//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElement.h"
#include "2D/BsImageSprite.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/** Flags that control how does a slider handle behave. */
	enum class GUISliderHandleFlag
	{
		/** Slider handle will move horizontally. Cannot be used with the Vertical option. */
		Horizontal = 1 << 0,
		/** Slider handle will move vertically. Cannot be used with the Horizontal option. */
		Vertical = 1 << 1,
		/**
		 * If enabled, clicking on a specific slider position will cause the handle to jump to that position. If false the
		 * handle will only slightly move in that direction.
		 */
		JumpOnClick = 1 << 2,
		/** Determines should the slider handle provide additional side-handles that allow it to be resized. */
		Resizeable = 1 << 3
	};

	typedef Flags<GUISliderHandleFlag> GUISliderHandleFlags;
	BS_FLAGS_OPERATORS(GUISliderHandleFlag);

	/** A handle that can be dragged from its predefined minimum and maximum position, either horizontally or vertically. */
	class BS_EXPORT GUISliderHandle : public GUIElement
	{
		/**	Visual state of the handle. */
		enum class State
		{
			Normal, Hover, Active
		};

		/** State the handle can be in while user is dragging it. */
		enum class DragState
		{
			Normal, LeftResize, RightResize
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new handle.
		 *
		 * @param[in]	flags			Flags that control how does the handle behave.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHandle* create(GUISliderHandleFlags flags, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new handle.
		 *
		 * @param[in]	flags			Flags that control how does the handle behave.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHandle* create(GUISliderHandleFlags flags, const GUIOptions& options,
			const String& styleName = StringUtil::BLANK);

		/**	Gets the current position of the handle, in percent ranging [0.0f, 1.0f]. */
		float getHandlePos() const;

		/** Gets the minimum percentual variation of the handle position */
		float getStep() const;

		/**	Returns the position of the slider handle, in pixels. Relative to this object. */
		INT32 getHandlePosPx() const;

		/**	Returns remaining length of the scrollable area not covered by the handle, in pixels. */
		UINT32 getScrollableSize() const;

		/**	Returns the total length of the area the handle can move in, in pixels. */
		UINT32 getMaxSize() const;

		/**	
		 * Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no
		 * step. In percent.
		 */
		void setStep(float step);

		/**
		 * Moves the slider handle one step forwards or backwards. Step size is determined by step (if set) or handle size
		 * otherwise. If @p forward is true the handle is moved one step forward, otherwise one step backward.
		 */
		void moveOneStep(bool forward);

		/** Triggered when the user drags the handle. */
		Event<void(float pos, float size)> onHandleMovedOrResized;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/**
		 * Size of the handle in percent of the total draggable area, along the handle drag direction.
		 *
		 * @note	 Does not trigger layout update.
		 */
		void _setHandleSize(float pct);

		/**
		 * Moves the handle the the specified position in the handle area.
		 *
		 * @param[in]	pct	Position to move the handle to, in percent ranging [0.0f, 1.0f]
		 *
		 * @note	Does not trigger layout update.
		 */
		void _setHandlePos(float pct);

		/** Returns the size of the slider handle, in percent of the total area. */
		float _getHandleSizePct() const;

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */
	protected:
		~GUISliderHandle();

		/** @copydoc GUIElement::_fillBuffer() */
		void _fillBuffer(
			UINT8* vertices,
			UINT32* indices,
			UINT32 vertexOffset,
			UINT32 indexOffset,
			const Vector2I& offset,
			UINT32 maxNumVerts,
			UINT32 maxNumIndices,
			UINT32 renderElementIdx) const override;

		/** @copydoc GUIElement::updateRenderElementsInternal() */
		void updateRenderElementsInternal() override;

		/** @copydoc GUIElement::updateClippedBounds() */
		void updateClippedBounds() override;
	private:
		GUISliderHandle(GUISliderHandleFlags flags, const String& styleName, const GUIDimensions& dimensions);

		/** @copydoc GUIElement::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** Checks are the specified over the scroll handle. Coordinates are relative to the parent widget. */
		bool isOnHandle(const Vector2I& pos) const;

		/**	Sets the position of the slider handle, in pixels. Relative to this object. */
		void setHandlePosPx(INT32 pos);

		/** Returns the size of the handle button, in pixels. */
		UINT32 getHandleSize() const;

		/**	Gets the currently active texture, depending on handle state. */
		const HSpriteTexture& getActiveTexture() const;

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		static const UINT32 RESIZE_HANDLE_SIZE;

		ImageSprite* mImageSprite;

		GUISliderHandleFlags mFlags;
		UINT32 mMinHandleSize = 0;
		float mPctHandlePos = 0.0f;
		float mPctHandleSize = 0.0f;
		float mStep = 0.0f;
		INT32 mDragStartPos = 0;
		DragState mDragState = DragState::Normal;
		bool mMouseOverHandle = false;
		bool mHandleDragged = false;
		State mState = State::Normal;
	};

	/** @} */
}
