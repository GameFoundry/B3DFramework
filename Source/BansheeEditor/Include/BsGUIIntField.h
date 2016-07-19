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
	 * Label is optional. This specific implementation displays a integer input field.
	 */
	class BS_ED_EXPORT GUIIntField : public TGUIField<GUIIntField>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Style type name for the internal input box. */
		static const String& getInputStyleType();

		GUIIntField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the value of the input field. */
		INT32 getValue() const { return mValue; }

		/**	Sets a new value in the input field, it returns the clamped value according to range and step. */
		INT32 setValue(INT32 value);

		/**
		 * Sets a minimum and maximum allow values in the input field. Set to large negative/positive values if you don't
		 * require clamping.
		 */
		void setRange(INT32 min, INT32 max);

		/**	Sets the minimum change allowed for the input field. */
		void setStep(INT32 step);

		/** Returns the minimum change allowed for the input field. */
		INT32 getStep() const { return mStep; }

		/**	Checks is the input field currently active. */
		bool hasInputFocus() const { return mHasInputFocus; }

		/** @copydoc GUIElement::setTint */
		void setTint(const Color& color) override;

		Event<void(INT32)> onValueChanged; /**< Triggers when the internal value changes. */
		Event<void()> onConfirm; /**< Triggered when the user hits the Enter key with the input box in focus. */

		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets a new value in the input field, and also allows you to choose should the field trigger an onValueChanged 
		 * event.
		 */
		void _setValue(INT32 value, bool triggerEvent);

		/** @} */
	protected:
		virtual ~GUIIntField();

		/** @copydoc GUIElement::_hasCustomCursor */
		bool _hasCustomCursor(const Vector2I position, CursorType& type) const override;

		/** @copydoc GUIElement::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev)  override;

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when the input box value changes. */
		void valueChanged(const WString& newValue);

		/**
		 * Triggered when the input box value changes, but unlike the previous overload the value is parsed into an integer
		 * value.
		 */
		void valueChanged(INT32 newValue);

		/**	Triggers when the input box receives or loses keyboard focus. */
		void focusChanged(bool focus);

		/**	Triggered when the users confirms input in the input box. */
		void inputConfirmed();

		/**	Callback that checks can the provided string be converted to an integer value. */
		static bool intFilter(const WString& str);

		static const INT32 DRAG_SPEED;

		GUIInputBox* mInputBox;
		INT32 mValue;
		INT32 mLastDragPos;
		INT32 mMinValue;
		INT32 mMaxValue;
		INT32 mStep;
		bool mIsDragging;
		bool mIsDragCursorSet;
		bool mHasInputFocus;
	};

	/** @} */
}