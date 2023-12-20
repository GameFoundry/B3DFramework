//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsGUIContent.h"
#include "BsGUISpriteHelper.h"
#include "BsPrerequisites.h"
#include "GUI/BsGUIInteractable.h"
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
	B3D_FLAGS_OPERATORS(GUISliderHandleFlag);

	/** A handle that can be dragged from its predefined minimum and maximum position, either horizontally or vertically. */
	class B3D_EXPORT GUISliderHandle : public GUIInteractable
	{
		/** State the handle can be in while user is dragging it. */
		enum class DragState
		{
			Normal,
			LeftResize,
			RightResize
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new handle.
		 *
		 * @param[in]	flags			Flags that control how does the handle behave.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHandle* Create(GUISliderHandleFlags flags, const String& styleName = StringUtil::kBlank);

		/**
		 * Creates a new handle.
		 *
		 * @param[in]	flags			Flags that control how does the handle behave.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHandle* Create(GUISliderHandleFlags flags, const GUIOptions& options, const String& styleName = StringUtil::kBlank);

		const char* GetStyleSheetElement() const override { return "slider-handle"; }

		/**	Gets the current position of the handle, in percent ranging [0.0f, 1.0f]. */
		float GetHandlePositionInPercent() const { return mHandlePositionInPercent; }

		/** Gets the minimum percentual variation of the handle position */
		float GetMinimumStepIncrement() const { return mMinimumStepIncrement; }

		/**	Returns the position of the slider handle, in pixels. Relative to this object. */
		i32 GetHandlePositionInPixels() const;

		/**	Returns remaining length of the scrollable area not covered by the handle, in pixels. */
		u32 GetScrollableLength() const { return GetTotalLength() - GetHandleSizeInPixels(); }

		/**	Returns the total length of the area the handle can move in, in pixels. */
		u32 GetTotalLength() const;

		/**
		 * Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no
		 * step. In percent.
		 */
		void SetMinimumStepIncrement(float step) { mMinimumStepIncrement = Math::Clamp01(step); }

		/**
		 * Moves the slider handle one step forwards or backwards. Step size is determined by step (if set) or handle size
		 * otherwise. If @p forward is true the handle is moved one step forward, otherwise one step backward.
		 */
		void MoveOneStep(bool forward);

		/** Triggered when the user drags the handle. */
		Event<void(float position, float size)> OnHandleMovedOrResized;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Returns the size of the slider handle, in percent of the total area. */
		float GetHandleSizeInPercent() const { return mHandleSizeInPercent; }

		/** Returns the size of the handle button, in pixels. */
		u32 GetHandleSizeInPixels() const;

		/**
		 * Size of the handle in percent of the total draggable area, along the handle drag direction.
		 *
		 * @param	percent		Size of the handle, in percent ranging [0.0f, 1.0f]
		 *
		 * @note	 Does not trigger layout update.
		 */
		void SetHandleSizeInPercent(float percent) { mHandleSizeInPercent = Math::Clamp01(percent); }

		/**
		 * Moves the handle the the specified position in the handle area.
		 *
		 * @param	percent		Position to move the handle to, in percent ranging [0.0f, 1.0f]
		 *
		 * @note	Does not trigger layout update.
		 */
		void SetHandlePositionInPercent(float percent);

		/**	Sets the position of the slider handle, in pixels. Relative to this object. */
		void SetHandlePositionInPixels(i32 position);

		Vector2I CalculateUnconstrainedOptimalSize() const override;

		/** @} */
	protected:
		void UpdateRenderElements() override;

		static constexpr u32 kMinimumHandleSize = 5;
	private:
		GUISliderHandle(GUISliderHandleFlags flags, const String& styleName, const GUISizeConstraints& dimensions);

		bool DoOnMouseEvent(const GUIMouseEvent& ev) override;

		/** Checks are the specified over the scroll handle. Coordinates are relative to the parent widget. */
		bool IsOnHandle(const Vector2I& position) const;

		GUIBackgroundSprite mBackgroundSprite;

		GUISliderHandleFlags mFlags;
		float mHandlePositionInPercent = 0.0f;
		float mHandleSizeInPercent = 0.0f;
		float mMinimumStepIncrement = 0.0f;
		i32 mDragStartPos = 0;
		DragState mDragState = DragState::Normal;
		bool mMouseOverHandle = false;
		bool mHandleDragged = false;
		GUIElementState mState = GUIElementState::Normal;
	};

	/** @} */
} // namespace bs
