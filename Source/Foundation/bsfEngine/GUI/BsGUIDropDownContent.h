//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsGUIElementContainer.h"
#include "GUI/BsGUIDropDownMenu.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	GUI element that is used for representing entries in a drop down menu. */
	class BS_EXPORT GUIDropDownContent : public GUIElementContainer
	{
		/**	Contains various GUI elements used for displaying a single menu entry. */
		struct VisibleElement
		{
			UINT32 idx = 0;
			GUIButtonBase* button = nullptr;
			GUITexture* separator = nullptr;
			GUILabel* shortcutLabel = nullptr;
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& getGUITypeName();

		/**
		 * Creates a new drop down contents element.
		 *
		 * @param[in]	parent			Parent sub-menu that owns the drop down contents.
		 * @param[in]	dropDownData	Data that will be used for initializing the child entries.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIDropDownContent* create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData,
			const String& style = StringUtil::BLANK);

		/**
		 * Creates a new drop down contents element.
		 *
		 * @param[in]	parent			Parent sub-menu that owns the drop down contents.
		 * @param[in]	dropDownData	Data that will be used for initializing the child entries.
		 * @param[in]	options			Options that allow you to control how is the element positioned and sized.
		 *								This will override any similar options set by style.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIDropDownContent* create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, 
			const GUIOptions& options, const String& style = StringUtil::BLANK);

		/**
		 * Changes the range of the displayed elements. 
		 *
		 * @note	This must be called at least once after creation.
		 */
		void setRange(UINT32 start, UINT32 end);

		/**	Returns height of a menu element at the specified index, in pixels. */
		UINT32 getElementHeight(UINT32 idx) const;

		/**
		 * Enables or disables keyboard focus. When keyboard focus is enabled the contents will respond to keyboard events.
		 */
		void setKeyboardFocus(bool focus);

		static constexpr const char* ENTRY_TOGGLE_STYLE_TYPE = "DropDownEntryToggleBtn";
		static constexpr const char* ENTRY_STYLE_TYPE = "DropDownEntryBtn";
		static constexpr const char* ENTRY_EXP_STYLE_TYPE = "DropDownEntryExpBtn";
		static constexpr const char* SEPARATOR_STYLE_TYPE = "DropDownSeparator";
	protected:
		GUIDropDownContent(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, 
			const String& style, const GUIDimensions& dimensions);

		/**	Get localized name of a menu item element with the specified index. */
		HString getElementLocalizedName(UINT32 idx) const;

		/** @copydoc GUIElementContainer::_getOptimalSize */
		Vector2I _getOptimalSize() const override;

		/** @copydoc GUIElementContainer::_updateLayoutInternal */
		void _updateLayoutInternal(const GUILayoutData& data) override;

		/** @copydoc GUIElementContainer::styleUpdated */
		void styleUpdated() override;

		/** @copydoc GUIElementContainer::_commandEvent */
		bool _commandEvent(const GUICommandEvent& ev) override;

		/** @copydoc GUIElementContainer::_mouseEvent */
		bool _mouseEvent(const GUIMouseEvent& ev) override;

		/**
		 * Marks the element with the specified index as selected.
		 * 		
		 * @param[in]	idx		Index of the displayed element (indexing visible elements).
		 */
		void setSelected(UINT32 idx);

		/**
		 * Selects the next available non-separator entry.
		 * 			
		 * @param[in]	startIdx	Index of the menu element.
		 */
		void selectNext(UINT32 startIdx);

		/**
		 * Selects the previous available non-separator entry.
		 * 			
		 * @param[in]	startIdx	Index of the menu element.
		 */
		void selectPrevious(UINT32 startIdx);

		GUIDropDownData mDropDownData;
		Vector<bool> mStates;
		Vector<VisibleElement> mVisibleElements;
		UINT32 mSelectedIdx;
		UINT32 mRangeStart, mRangeEnd;
		GUIDropDownMenu::DropDownSubMenu* mParent;
		bool mKeyboardFocus;
		bool mIsToggle;
	};

	/** @} */
}