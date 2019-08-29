//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "GUI/BsCGUIWidget.h"
#include "Math/BsRect2I.h"
#include "GUI/BsDropDownAreaPlacement.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains items used for initializing one level in a drop down box hierarchy. */
	struct BS_EXPORT GUIDropDownData
	{
		Vector<GUIDropDownDataEntry> entries;
		Vector<bool> states;
		UnorderedMap<String, HString> localizedNames;
	};

	/**	A set of parameters used for initializing a drop down box. */
	struct DROP_DOWN_BOX_DESC
	{
		SPtr<Camera> camera; /**< Camera on which to open the drop down box. */
		DropDownAreaPlacement placement; /**< Determines how is the drop down box positioned in the visible area. */
		GUIDropDownData dropDownData; /**< Data to use for initializing menu items of the drop down box. */
		HGUISkin skin; /**< Skin to use for drop down box GUI elements. */
		/** Additional bounds that control what is considered the inside or the outside of the drop down box. */
		Vector<Rect2I> additionalBounds;
	};

	/**	Represents a single entry in a drop down box. */
	class BS_EXPORT GUIDropDownDataEntry
	{
		enum class Type
		{
			Separator,
			Entry,
			SubMenu
		};

	public:
		/**	Creates a new separator entry. */
		static GUIDropDownDataEntry separator();

		/** Creates a new button entry with the specified callback that is triggered when button is selected. */
		static GUIDropDownDataEntry button(const String& label, std::function<void()> callback,
			const String& shortcutTag = StringUtil::BLANK);

		/** Creates a new sub-menu entry that will open the provided drop down data sub-menu when activated. */
		static GUIDropDownDataEntry subMenu(const String& label, const GUIDropDownData& data);

		/**	Check is the entry a separator. */
		bool isSeparator() const { return mType == Type::Separator; }

		/**	Check is the entry a sub menu. */
		bool isSubMenu() const { return mType == Type::SubMenu; }

		/**	Returns display label of the entry (if an entry is a button or a sub-menu). */
		const String& getLabel() const { return mLabel; }

		/**	Returns the shortcut key combination string that is to be displayed along the entry label. */
		const String& getShortcutTag() const { return mShortcutTag; }

		/**	Returns a button callback if the entry (if an entry is a button). */
		std::function<void()> getCallback() const { return mCallback; }

		/**	Returns sub-menu data that is used for creating a sub-menu (if an entry is a sub-menu). */
		const GUIDropDownData& getSubMenuData() const { return mChildData; }
	private:
		GUIDropDownDataEntry() { }

		std::function<void()> mCallback;
		GUIDropDownData mChildData;
		String mLabel;
		String mShortcutTag;
		Type mType;
	};

	/**	Type of drop down box types. */
	enum class GUIDropDownType
	{
		ListBox,
		MultiListBox,
		ContextMenu,
		MenuBar
	};

	/**	This is a generic GUI drop down box class that can be used for: list boxes, menu bars or context menus. */
	class BS_EXPORT GUIDropDownMenu : public CGUIWidget
	{
	public:
		/**
		 * Creates a new drop down box widget.
		 *
		 * @param[in]	parent	Parent scene object to attach the drop down box to.
		 * @param[in]	desc	Various parameters that control the drop down menu features and content.
		 * @param[in]	type	Specific type of drop down box to display.
		 */
		GUIDropDownMenu(const HSceneObject& parent, const DROP_DOWN_BOX_DESC& desc, GUIDropDownType type);
		~GUIDropDownMenu();

	private:
		/**	Contains data about a single drop down box sub-menu. */
		struct DropDownSubMenu
		{
			/**	Represents a single sub-menu page. */
			struct PageInfo
			{
				UINT32 idx;
				UINT32 start;
				UINT32 end;
				UINT32 height;
			};

		public:
			/**
			 * Creates a new drop down box sub-menu.
			 *
			 * @param[in]	owner			Owner drop down box this sub menu belongs to.
			 * @param[in]	parent			Parent sub-menu. Can be null.
			 * @param[in]	placement		Determines how is the sub-menu positioned in the visible area.
			 * @param[in]	availableBounds	Available bounds (in pixels) in which the sub-menu may be opened.
			 * @param[in]	dropDownData	Data to use for initializing menu items of the sub-menu.
			 * @param[in]	type			Type of the drop down box to show.
			 * @param[in]	depthOffset		How much to offset the sub-menu depth. We want deeper levels of the sub-menu
			 *								hierarchy to be in front of lower levels, so you should increase this value for
			 *								each level of the sub-menu hierarchy.
			 */
			DropDownSubMenu(GUIDropDownMenu* owner, DropDownSubMenu* parent, const DropDownAreaPlacement& placement,
				const Rect2I& availableBounds, const GUIDropDownData& dropDownData, GUIDropDownType type, UINT32 depthOffset);
			~DropDownSubMenu();

			/**	Recreates all internal GUI elements for the entries of the current sub-menu page. */
			void updateGUIElements();

			/**	Moves the sub-menu to the previous page and displays its elements, if available. */
			void scrollDown();

			/**	Moves the sub-menu to the next page and displays its elements, if available. */
			void scrollUp();

			/**	Moves the sub-menu to the first page and displays its elements. */
			void scrollToTop();

			/**	Moves the sub-menu to the last page and displays its elements. */
			void scrollToBottom();

			/**	Calculates ranges for all the pages of the sub-menu. */
			Vector<PageInfo> getPageInfos() const;

			/**
			 * Called when the user activates an element with the specified index.
			 *
			 * @param[in]	idx		Index of the activated element.
			 * @param[in]	bounds	Bounds of the GUI element that is used as a visual representation of this drop down
			 *						element.
			 */
			void elementActivated(UINT32 idx, const Rect2I& bounds);

			/**
			 * Called when the user selects an element with the specified index.
			 *
			 * @param[in]	idx		Index of the element that was selected.
			 */
			void elementSelected(UINT32 idx);

			/**	Called when the user wants to close the currently open sub-menu. */
			void closeSubMenu();

			/**	Closes this sub-menu. */
			void close();

			/**	Returns the type of the displayed drop down menu. */
			GUIDropDownType getType() const { return mType; }

			/**	Returns actual visible bounds of the sub-menu. */
			Rect2I getVisibleBounds() const { return mVisibleBounds; }

			/**	Returns the drop box object that owns this sub-menu. */
			GUIDropDownMenu* getOwner() const { return mOwner; }

		public:
			GUIDropDownMenu* mOwner;

			GUIDropDownType mType;
			GUIDropDownData mData;
			UINT32 mPage;
			INT32 x, y;
			UINT32 width, height;
			Rect2I mVisibleBounds;
			Rect2I mAvailableBounds;
			UINT32 mDepthOffset;
			bool mOpenedUpward;

			GUIDropDownContent* mContent;
			GUITexture* mBackgroundFrame;
			GUIButton* mScrollUpBtn;
			GUIButton* mScrollDownBtn;
			GUITexture* mHandle;

			GUIPanel* mBackgroundPanel;
			GUIPanel* mContentPanel;
			GUILayout* mContentLayout;
			GUIPanel* mSidebarPanel;

			DropDownSubMenu* mParent;
			DropDownSubMenu* mSubMenu;
		};

	private:
		friend class GUIDropDownContent;

		/**	Called when the specified sub-menu is opened. */
		void notifySubMenuOpened(DropDownSubMenu* subMenu);

		/**	Called when the specified sub-menu is opened. */
		void notifySubMenuClosed(DropDownSubMenu* subMenu);

		/**	Called when the drop down box loses focus (and should be closed). */
		void dropDownFocusLost();

		/** @copydoc CGUIWidget::onDestroyed */
		void onDestroyed() override;

	private:
		static const UINT32 DROP_DOWN_BOX_WIDTH;

		String mScrollUpStyle;
		String mScrollDownStyle;
		String mBackgroundStyle;
		String mContentStyle;
		String mSideBackgroundStyle;
		String mHandleStyle;

		DropDownSubMenu* mRootMenu;
		GUIDropDownHitBox* mFrontHitBox;
		GUIDropDownHitBox* mBackHitBox;

		// Captures mouse clicks so that we don't trigger elements outside the drop down box when we just want to close it.
		// (Particular example is clicking on the button that opened the drop down box in the first place. Clicking will cause
		// the drop down to lose focus and close, but if the button still processes the mouse click it will be immediately opened again)
		GUIDropDownHitBox* mCaptureHitBox;

		Vector<Rect2I> mAdditionalCaptureBounds;
	};

	/** @} */
}
