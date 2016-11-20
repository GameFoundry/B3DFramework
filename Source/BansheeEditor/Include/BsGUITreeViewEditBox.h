//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsGUIInputBox.h"

namespace bs
{
	/** @addtogroup GUI-Editor-Internal
	 *  @{
	 */

	/** Input box used for renaming elements in a TreeView. */
	class GUITreeViewEditBox : public GUIInputBox
	{
	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new GUI tree view edito box element.
		 *
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITreeViewEditBox* create(const String& styleName = StringUtil::BLANK);

		/**
		 * Creates a new GUI tree view edito box element.
		 *
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	styleName		Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default style is used.
		 */
		static GUITreeViewEditBox* create(const GUIOptions& options, const String& styleName = StringUtil::BLANK);

		Event<void()> onInputConfirmed; /**< Triggered when the user confirms the input in the edit box. */
		Event<void()> onInputCanceled; /**< Triggered when the user cancels the input in the edit box. */
		Event<void()> onFocusLost; /**< Triggered when the user clicks outside of the editor box. */
	private:
		GUITreeViewEditBox(const String& styleName, const GUIDimensions& dimensions);

		/** @copydoc GUIElement::_commandEvent */
		virtual bool _commandEvent(const GUICommandEvent& ev) override;
	};

	/** @} */
}