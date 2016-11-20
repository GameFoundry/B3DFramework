//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIFieldBase.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * A composite GUI object representing an editor field. Editor fields are a combination of a label and an input field.
	 * Label is optional. This specific implementation displays a toggle button.
	 */
	class BS_ED_EXPORT GUIToggleField : public TGUIField<GUIToggleField>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Returns style type name of the internal GUIToggle element. */
		static const String& getToggleStyleType();

		GUIToggleField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the current value of the toggle. */
		bool getValue() const { return mValue; }

		/**	Activates/deactivates the toggle. */
		void setValue(bool value);

		/** @copydoc GUIElement::setTint */
		void setTint(const Color& color) override;

		Event<void(bool)> onValueChanged; /** Triggered when the value of the toggle changes. */

		/** @name Internal 
		 *  @{
		 */

		/**
		 * Sets a new value in the input field, and also allows you to choose should the field trigger an onValueChanged
		 * event.
		 */
		void _setValue(bool value, bool triggerEvent);

		/** @} */
	protected:
		virtual ~GUIToggleField() { }

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when the value of the internal toggle button changes. */
		void valueChanged(bool newValue);

		GUIToggle* mToggle;
		bool mValue;
	};

	/** @} */
}