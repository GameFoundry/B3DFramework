//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIButtonBase.h"
#include "GUI/BsGUIToggleGroup.h"
#include "2D/BsImageSprite.h"
#include "2D/BsTextSprite.h"
#include "GUI/BsGUIContent.h"
#include "Utility/BsEvent.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	GUI element representing a toggle (on/off) button. */
	class BS_EXPORT GUIToggle : public GUIButtonBase
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	text			Label to display in the button, if any.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const HString& text, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	text			Label to display in the button, if any.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const HString& text, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);
	
		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	text			Label to display in the button, if any.
		 * @param[in]	toggleGroup		Toggle group this button belongs to.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const HString& text, SPtr<GUIToggleGroup> toggleGroup,
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	text			Label to display in the button, if any.
		 * @param[in]	toggleGroup		Toggle group this button belongs to.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const HString& text, SPtr<GUIToggleGroup> toggleGroup,
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	content			Content to display in the button, if any.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const GUIContent& content, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	content			Content to display in the button, if any.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const GUIContent& content, const GUIOptions& options, 
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	content			Content to display in the button, if any.
		 * @param[in]	toggleGroup		Toggle group this button belongs to.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup,
			const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new toggle button with the specified label.
		 *
		 * @param[in]	content			Content to display in the button, if any.
		 * @param[in]	toggleGroup		Toggle group this button belongs to.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUIToggle* create(const GUIContent& content, SPtr<GUIToggleGroup> toggleGroup,
			const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a toggle group that you may provide to GUIToggle upon construction. Toggles sharing the same group will
		 * only have a single element active at a time.
		 *
		 * @param[in]	allowAllOff	If true all of the toggle buttons can be turned off, if false one will always be turned 
		 *							on.
		 */
		static SPtr<GUIToggleGroup> createToggleGroup(bool allowAllOff = false);

		/**	Checks the toggle, making it active. */
		void toggleOn() { _toggleOn(true); }

		/**	Unchecks the toggle, making it inactive. */
		void toggleOff() { _toggleOff(true); }

		/**	Checks is the toggle currently on. */
		bool isToggled() const { return mIsToggled; }

		/**	Triggered whenever the button is toggled on or off. */
		Event<void(bool)> onToggled;

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** @copydoc GUIButtonBase::_getElementType */
		ElementType _getElementType() const override { return ElementType::Toggle; }

		/** Sets a toggle group of the toggle button. Toggling one button in a group will automatically untoggle others. */
		void _setToggleGroup(SPtr<GUIToggleGroup> toggleGroup);

		/**	Checks the toggle, making it active and optionally triggering the onToggled event. */
		virtual void _toggleOn(bool triggerEvent);

		/**	Unchecks the toggle, making it inactive and optionally triggering the onToggled event. */
		virtual void _toggleOff(bool triggerEvent);

		/** @} */
	protected:
		virtual ~GUIToggle();

	protected:
		GUIToggle(const String& styleName, const GUIContent& content, 
			SPtr<GUIToggleGroup> toggleGroup, const GUIDimensions& dimensions);

		/** @copydoc GUIButtonBase::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/** @copydoc GUIButtonBase::_commandEvent */
		bool _commandEvent(const GUICommandEvent& ev) override;

	protected:
		SPtr<GUIToggleGroup> mToggleGroup;
		bool mIsToggled;
	};

	/** @} */
}