//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementContainer.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	A slider with a draggable handle that can be vertical or horizontal. */
	class BS_EXPORT GUISlider : public GUIElementContainer
	{
	public:
		/**	Name of the style for the handle button used by the slider. */
		static const String& GetHandleStyleType();

		/**	Name of the style for the background image used by the slider. */
		static const String& GetBackgroundStyleType();

		/**	Name of the style for the background fill image used by the slider. */
		static const String& GetFillStyleType();

		/**
		 * Moves the slider handle the the specified position in the handle area.
		 *
		 * @param[in]	pct	Position to move the handle to, in percent ranging [0.0f, 1.0f]
		 */
		void SetPercent(float pct);

		/**	Gets the current position of the slider handle, in percent ranging [0.0f, 1.0f]. */
		float GetPercent() const;

		/**
		 * Gets the current value of the slider. This is the slider handle position percentage scaled within the current
		 * minimum and maximum range, rounded up to nearest step increment.
		 */
		float GetValue() const;

		/**	Sets a new value of the slider. This value should be within minimum and maximum range values. */
		void SetValue(float value);

		/**
		 * Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don't
		 * require clamping.
		 */
		void SetRange(float min, float max);

		/** Returns the minimum value of the slider */
		float GetRangeMinimum() const;

		/** Returns the maximum value of the slider */
		float GetRangeMaximum() const;

		/**
		 * Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no
		 * step.
		 */
		void SetStep(float step);

		/** Gets the minimum percentual variation of the handle position */
		float GetStep() const;

		/** @copydoc GUIElement::setTint */
		void SetTint(const Color& color);

		/** Triggered when the user changes the value of the slider. */
		Event<void(float percent)> OnChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementContainer::GetOptimalSizeInternal */
		Vector2I GetOptimalSizeInternal() const override;

		/** @} */
	protected:
		GUISlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUISlider();

		/** @copydoc GUIElementContainer::_updateLayoutInternal */
		void UpdateLayoutInternalInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElementContainer::styleUpdated */
		void StyleUpdated();

		/**	Triggered when the slider handles moves. */
		void OnHandleMoved(float newPosition, float newSize);

		/** @copydoc GUIElement::_commandEvent */
		bool CommandEventInternal(const GUICommandEvent& ev) override;

	private:
		GUISliderHandle* mSliderHandle;
		GUITexture* mBackground;
		GUITexture* mFillBackground;
		bool mHorizontal;
		float mMinRange = 0.0f;
		float mMaxRange = 1.0f;
		bool mHasFocus = false;

		HEvent mHandleMovedConn;
	};

	/** @} */

	/** @addtogroup GUI
	 *  @{
	 */

	/**	A horizontal slider with a draggable handle. */
	class BS_EXPORT GUISliderHorz : public GUISlider
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new horizontal slider.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHorz* Create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new horizontal slider.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHorz* Create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

	private:
		GUISliderHorz(const String& styleName, const GUIDimensions& dimensions);
	};

	/**	A vertical slider with a draggable handle. */
	class BS_EXPORT GUISliderVert : public GUISlider
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new vertical slider.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderVert* Create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new vertical slider.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderVert* Create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

	private:
		GUISliderVert(const String& styleName, const GUIDimensions& dimensions);
	};

	/** @} */
} // namespace bs
