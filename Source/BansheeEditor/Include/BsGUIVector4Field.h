//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIFieldBase.h"
#include "BsVector4.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * A composite GUI object representing an editor field. Editor fields are a combination of a label and an input field.
	 * Label is optional. This specific implementation displays a Vector4 input field.
	 */
	class BS_ED_EXPORT GUIVector4Field : public TGUIField<GUIVector4Field>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Style type name for the internal input boxes. */
		static const String& getFloatFieldStyleType();

		GUIVector4Field(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the value of the input field. */
		Vector4 getValue() const;

		/**	Sets a new value in the input field. */
		void setValue(const Vector4& value);

		/**	Checks is the input field currently active. */
		bool hasInputFocus() const;

		/** @copydoc GUIElement::setTint */
		virtual void setTint(const Color& color) override;

		Event<void(const Vector4&)> onValueChanged; /**< Triggers when the field value changes. */
		Event<void()> onConfirm; /**< Triggered when the user hits the Enter key with the input box in focus. */
	protected:
		virtual ~GUIVector4Field() { }

		/** @copydoc GUIElement::setTint */
		void styleUpdated() override;

		/**	Triggered when the values in any of the input boxes change. */
		void valueChanged(float newValue);

		/**	Triggered when the users confirms input in the input box. */
		void inputConfirmed();

		static const UINT32 ELEMENT_LABEL_WIDTH;

		GUIFloatField* mFieldX;
		GUIFloatField* mFieldY;
		GUIFloatField* mFieldZ;
		GUIFloatField* mFieldW;
	};

	/** @} */
}