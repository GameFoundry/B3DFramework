//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIElementContainer.h"

namespace bs
{
	/** @addtogroup GUI-Editor
	 *  @{
	 */

	/**
	 * A composite GUI object representing an editor field. Editor fields are a combination of a label and an input field.
	 * Label is optional. This specific implementation displays a text input field.
	 */
	class BS_ED_EXPORT GUITextField : public GUIElementContainer
	{
		struct PrivatelyConstruct {};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles. */
		static const String& getGUITypeName();

		/** Returns style type name of the internal GUILabel element. */
		static const String& getLabelStyleType();

		/** Returns style type name of the internal GUIInputBox element. */
		static const String& getInputStyleType();

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized. This will
		 *								override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const GUIContent& labelContent, UINT32 labelWidth, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const GUIContent& labelContent, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelText		String to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const HString& labelText, UINT32 labelWidth, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelText		String to display in the editor field label.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const HString& labelText, const GUIOptions& options,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field without a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelContent	Content to display in the editor field label.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const GUIContent& labelContent,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelText		String to display in the editor field label.
		 * @param[in]	labelWidth		Width of the label in pixels.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const HString& labelText, UINT32 labelWidth,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field with a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	labelText		String to display in the editor field label.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const HString& labelText,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new GUI text editor field without a label.
		 *
		 * @param[in]	multiline		Should the input box accept multiple lines of input.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITextField* create(bool multiline, const String& style = StringUtil::BLANK);

		GUITextField(const PrivatelyConstruct& dummy, bool multiline, const GUIContent& labelContent, UINT32 labelWidth,
			const String& style, const GUIDimensions& dimensions, bool withLabel);

		/**	Returns the value currently input in the text field. */
		WString getValue() const { return mValue; }

		/**	Sets a new value in the text field. */
		void setValue(const WString& value);

		/**	Checks whether the element currently has keyboard focus. */
		bool hasInputFocus() const { return mHasInputFocus; }

		/** @copydoc GUIElement::setTint */
		void setTint(const Color& color) override;
		
		Event<void(const WString&)> onValueChanged; /** Triggered when the value in the field changes. */
		Event<void()> onConfirm; /**< Triggered when the user hits the Enter key with the input box in focus. */

		/** @name Internal
		 *  @{
		 */

		/**
		 * Sets a new value in the input field, and also allows you to choose should the field trigger an onValueChanged
		 * event.
		 */
		void _setValue(const WString& value, bool triggerEvent);

		/** @} */
	protected:
		static const UINT32 DEFAULT_LABEL_WIDTH;

		virtual ~GUITextField();

		/** @copydoc GUIElement::styleUpdated */
		void styleUpdated() override;

		/** @copydoc GUIElement::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElement::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/**	Triggered when the value in the internal input box changes. */
		void valueChanged(const WString& newValue);

		/**	Triggers when the input box receives or loses keyboard focus. */
		void focusChanged(bool focus);

		/**	Triggered when the users confirms input in the input box. */
		void inputConfirmed();

		GUIInputBox* mInputBox;
		GUILayout* mLayout;
		GUILabel* mLabel;
		bool mHasInputFocus;
		WString mValue;
	};

	/** @} */
}