//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsGUIButtonBase.h"
#include "BsGUIToggleGroup.h"
#include "BsImageSprite.h"
#include "BsTextSprite.h"
#include "BsGUIContent.h"
#include "BsEvent.h"

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
		virtual void toggleOn();

		/**	Unchecks the toggle, making it inactive. */
		virtual void toggleOff();

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

		/** @} */
	protected:
		virtual ~GUIToggle();

	protected:
		GUIToggle(const String& styleName, const GUIContent& content, 
			SPtr<GUIToggleGroup> toggleGroup, const GUIDimensions& dimensions);

		/** @copydoc GUIButtonBase::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

	protected:
		SPtr<GUIToggleGroup> mToggleGroup;
		bool mIsToggled;
	};

	/** @} */
}