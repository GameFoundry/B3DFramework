//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIButtonBase.h"
#include "BsGUIContent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** GUI button that can be clicked. Has normal, hover and active states with an optional label. */
	class BS_EXPORT GUIButton : public GUIButtonBase
	{
	public:
		/**
		 * Returns type name of the GUI element used for finding GUI element styles. 
		 */
		static const String& getGUITypeName();

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param[in]	text		Label to display on the button.
		 * @param[in]	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *							GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIButton* create(const HString& text, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param[in]	text			Label to display on the button.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. 
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIButton* create(const HString& text, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param[in]	content		Content to display on a button. May include a label, image and a tooltip.
		 * @param[in]	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the 
		 *							GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIButton* create(const GUIContent& content, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new button with the specified label.
		 *
		 * @param[in]	content		Content to display on a button. May include a label, image and a tooltip.
		 * @param[in]	options		Options that allow you to control how is the element positioned and sized. This will 
		 *							override any similar options set by style.
		 * @param[in]	styleName	Optional style to use for the element. Style will be retrieved from GUISkin of the 
		 *							GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIButton* create(const GUIContent& content, const GUIOptions& options, const String& styleName = StringUtil::BLANK);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIElement::_getElementType */
		ElementType _getElementType() const override { return ElementType::Button; }

		/** @} */
	private:
		GUIButton(const String& styleName, const GUIContent& content, const GUIDimensions& dimensions);
	};

	/** @} */
}