//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIFieldBase.h"

namespace BansheeEngine
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * A composite GUI object representing an editor field. Editor fields are a combination of a label and an input field.
	 * Label is optional. This specific implementation displays a horizontal slider and a floating point input box.
	 */
	class BS_ED_EXPORT GUISliderField : public TGUIField<GUISliderField>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Style type name for the internal input box. */
		static const String& getInputStyleType();

		/** Style type name for the internal slider. */
		static const String& getSliderStyleType();

		GUISliderField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the value of the input field/slider. */
		float getValue() const;

		/** Gets the minimum percentual variation of the handle position */
		float getStep() const;

		/**	Sets a new value in the input field/slider, it returns the clamped value according to range and step. */
		float setValue(float value);

		/**
		 * Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don't
		 * require clamping.
		 */
		void setRange(float min, float max);

		/**
		 * Sets a step that defines the minimal increment the value can be increased/decreased by. Set to zero to have no
		 * step.
		 */
		void setStep(float step);

		/**	Checks is the input field currently active. */
		bool hasInputFocus() const { return mHasInputFocus; }

		/** @copydoc GUIElement::setTint */
		void setTint(const Color& color) override;

		Event<void(float)> onValueChanged; /**< Triggers when the field value changes. */

		/** @name Internal 
		 *  @{
		 */

		/**
		 * Sets a new value in the input field, and also allows you to choose should the field trigger an onValueChanged
		 * event.
		 */
		void _setValue(float value, bool triggerEvent);

		/** @} */
	protected:
		virtual ~GUISliderField();

		/** @copydoc GUIElementContainer::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when the input box value changes definitively. */
		void inputBoxValueChanged(bool confirmed = true);

		/**	Triggered when the input box value is changing. */
		void inputBoxValueChanging(const WString& newValue);

		/**	Triggered when the slider is moved. */
		void sliderChanged(float newValue);

		/**	Triggers when the input box receives or loses keyboard focus. */
		void inputBoxFocusChanged(bool focus);

		/**	Callback that checks can the provided string be	converted to a floating point value. */
		static bool floatFilter(const WString& str);

		GUIInputBox* mInputBox;
		GUISliderHorz* mSlider;
		bool mHasInputFocus;
	};

	/** @} */
}