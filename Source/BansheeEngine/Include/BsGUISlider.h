//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIElementContainer.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/** @addtogroup Implementation
	 *  @{
	 */

	/**	A slider with a draggable handle that can be vertical or horizontal. */
	class BS_EXPORT GUISlider : public GUIElementContainer
	{
	public:
		/**	Name of the style for the handle button used by the slider. */
		static const String& getHandleStyleType();

		/**	Name of the style for the background image used by the slider. */
		static const String& getBackgroundStyleType();

		/**	Name of the style for the background fill image used by the slider. */
		static const String& getFillStyleType();

		/**
		 * Moves the slider handle the the specified position in the handle area.
		 *
		 * @param[in]	pct	Position to move the handle to, in percent ranging [0.0f, 1.0f]
		 */
		void setPercent(float pct);

		/**	Gets the current position of the slider handle, in percent ranging [0.0f, 1.0f]. */
		float getPercent() const;

		/**
		 * Gets the current value of the slider. This is the slider handle position percentage scaled within the current
		 * minimum and maximum range, rounded up to nearest step increment.
		 */
		float getValue() const;

		/**	Sets a new value of the slider. This value should be within minimum and maximum range values. */
		void setValue(float value);

		/**
		 * Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don't
		 * require clamping.
		 */
		void setRange(float min, float max);

		/** Returns the minimum value of the slider */
		float getRangeMinimum() const;

		/** Returns the maximum value of the slider */
		float getRangeMaximum() const;

		/**
		 * Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no
		 * step.
		 */
		void setStep(float step);

		/** Gets the minimum percentual variation of the handle position */
		float getStep() const;

		/** @copydoc GUIElement::setTint */
		void setTint(const Color& color) override;

		/** Triggered when the user changes the value of the slider. */
		Event<void(float percent)> onChanged;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElementContainer::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */
	protected:
		GUISlider(bool horizontal, const String& styleName, const GUIDimensions& dimensions);
		virtual ~GUISlider();

		/** @copydoc GUIElementContainer::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElementContainer::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when the slider handles moves. */
		void onHandleMoved(float newPosition, float newSize);

	private:
		GUISliderHandle* mSliderHandle;
		GUITexture* mBackground;
		GUITexture* mFillBackground;
		bool mHorizontal;
		float mMinRange;
		float mMaxRange;

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
		static const String& getGUITypeName();

		/**
		 * Creates a new horizontal slider.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHorz* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new horizontal slider.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderHorz* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

	private:
		GUISliderHorz(const String& styleName, const GUIDimensions& dimensions);
	};

	/**	A vertical slider with a draggable handle. */
	class BS_EXPORT GUISliderVert : public GUISlider
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new vertical slider.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderVert* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new vertical slider.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUISliderVert* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

	private:
		GUISliderVert(const String& styleName, const GUIDimensions& dimensions);
	};

	/** @} */
}