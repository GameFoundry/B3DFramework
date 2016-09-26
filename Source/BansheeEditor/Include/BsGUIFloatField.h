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
	 * Label is optional. This specific implementation displays a floating point input field.
	 */
	class BS_ED_EXPORT GUIFloatField : public TGUIField<GUIFloatField>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/** Style type name for the internal input box. */
		static const String& getInputStyleType();

		GUIFloatField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the value of the input field. */
		float getValue() const;

		/**	Sets a new value in the input field, it returns the clamped value according to range and step. */
		float setValue(float value);

		/**
		 * Sets a minimum and maximum allowed values in the input field. Set to large negative/positive values if you don't
		 * require clamping.
		 */
		void setRange(float min, float max);

		/**	Sets the minimum change allowed for the input field. */
		void setStep(float step);

		/** Returns the minimum change allowed for the input field. */
		float getStep() const{ return mStep; }

		/**	Checks is the input field currently active. */
		bool hasInputFocus() const { return mHasInputFocus; }

		/** @copydoc	GUIElement::setTint */
		void setTint(const Color& color) override;

		Event<void(float)> onValueChanged; /**< Triggers when the field value changes. */
		Event<void()> onConfirm; /**< Triggered when the user hits the Enter key with the input box in focus. */

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
		virtual ~GUIFloatField();

		/** @copydoc GUIElementContainer::_hasCustomCursor */
		bool _hasCustomCursor(const Vector2I position, CursorType& type) const override;

		/** @copydoc GUIElementContainer::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIElementContainer::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when the input box value changes. */
		void valueChanging(const WString& newValue);

		/** Triggered when the input box value changes and is confirmed. */
		void valueChanged(float newValue);

		/**	Triggers when the input box receives or loses keyboard focus. */
		void focusChanged(bool focus);

		/**	Triggered when the users confirms input in the input box. */
		void inputConfirmed();

		/** Updates the underlying input box with the text representing the provided floating point value. */
		void setText(float value);

		/** Clamps the provided value to current valid range, and step interval. */
		float applyRangeAndStep(float value) const;

		/** Callback that checks can the provided string be converted to a floating point value. */
		static bool floatFilter(const WString& str);

		static const float DRAG_SPEED;

		GUIInputBox* mInputBox;
		float mValue;
		INT32 mLastDragPos;
		float mMinValue;
		float mMaxValue;
		float mStep;
		bool mIsDragging;
		bool mHasInputFocus;
	};

	/** @} */
}