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
			u32 Idx = 0;
			GUIButtonBase* Button = nullptr;
			GUITexture* Separator = nullptr;
			GUILabel* ShortcutLabel = nullptr;
		};

	public:
		/** Returns type name of the GUI element used for finding GUI element styles.  */
		static const String& GetGuiTypeName();

		/**
		 * Creates a new drop down contents element.
		 *
		 * @param[in]	parent			Parent sub-menu that owns the drop down contents.
		 * @param[in]	dropDownData	Data that will be used for initializing the child entries.
		 * @param[in]	style			Optional style to use for the element. Style will be retrieved from GUISkin of the
		 *								GUIWidget the element is used on. If not specified default button style is used.
		 */
		static GUIDropDownContent* Create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const String& style = StringUtil::kBlank);

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
		static GUIDropDownContent* Create(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const GUIOptions& options, const String& style = StringUtil::kBlank);

		/**
		 * Changes the range of the displayed elements.
		 *
		 * @note	This must be called at least once after creation.
		 */
		void SetRange(u32 start, u32 end);

		/**	Returns height of a menu element at the specified index, in pixels. */
		u32 GetElementHeight(u32 idx) const;

		/**
		 * Enables or disables keyboard focus. When keyboard focus is enabled the contents will respond to keyboard events.
		 */
		void SetKeyboardFocus(bool focus);

		static constexpr const char* kEntryToggleStyleType = "DropDownEntryToggleBtn";
		static constexpr const char* kEntryStyleType = "DropDownEntryBtn";
		static constexpr const char* kEntryExpStyleType = "DropDownEntryExpBtn";
		static constexpr const char* kSeparatorStyleType = "DropDownSeparator";

	protected:
		GUIDropDownContent(GUIDropDownMenu::DropDownSubMenu* parent, const GUIDropDownData& dropDownData, const String& style, const GUIDimensions& dimensions);

		/**	Get localized name of a menu item element with the specified index. */
		HString GetElementLocalizedName(u32 idx) const;

		Vector2I GetOptimalSizeInternal() const override;
		void UpdateLayoutInternalInternal(const GUILayoutData& data) override;
		void StyleUpdated() override;
		bool CommandEventInternal(const GUICommandEvent& ev) override;
		bool MouseEventInternal(const GUIMouseEvent& ev) override;

		/**
		 * Marks the element with the specified index as selected.
		 *
		 * @param[in]	idx		Index of the displayed element (indexing visible elements).
		 */
		void SetSelected(u32 idx);

		/**
		 * Selects the next available non-separator entry.
		 *
		 * @param[in]	startIdx	Index of the menu element.
		 */
		void SelectNext(u32 startIdx);

		/**
		 * Selects the previous available non-separator entry.
		 *
		 * @param[in]	startIdx	Index of the menu element.
		 */
		void SelectPrevious(u32 startIdx);

		GUIDropDownData mDropDownData;
		Vector<bool> mStates;
		Vector<VisibleElement> mVisibleElements;
		u32 mSelectedIdx;
		u32 mRangeStart, mRangeEnd;
		GUIDropDownMenu::DropDownSubMenu* mParent;
		bool mKeyboardFocus;
		bool mIsToggle;
	};

	/** @} */
} // namespace bs
