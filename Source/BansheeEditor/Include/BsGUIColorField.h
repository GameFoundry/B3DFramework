//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIFieldBase.h"
#include "BsColor.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * A composite GUI object representing an editor field. Editor fields are a combination of a label and an input field.
	 * Label is optional. This specific implementation displays a color input field.
	 */
	class BS_ED_EXPORT GUIColorField : public TGUIField<GUIColorField>
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Style type name for the internal color field. */
		static const String& getColorInputStyleType();

		GUIColorField(const PrivatelyConstruct& dummy, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the value of the field. */
		Color getValue() const { return mValue; }

		/**	Changes the value of the field. */
		void setValue(const Color& value);

		/** @copydoc GUIElement::setTint */
		virtual void setTint(const Color& color) override;

		Event<void()> onClicked; /**< Triggered when the user clicks on the GUI element. */

		/** @name Internal 
		 *  @{
		 */

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @} */

	protected:
		virtual ~GUIColorField();

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		/**	Triggered when the child color input field is clicked on. */
		void clicked();

		UINT32 mLabelWidth;
		Color mValue;
		GUILabel* mLabel;
		GUIColor* mColor;
	};

	/** @} */
}