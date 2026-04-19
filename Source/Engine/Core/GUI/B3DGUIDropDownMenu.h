//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIWidget.h"
#include "Math/B3DArea2.h"
#include "GUI/B3DDropDownAreaPlacement.h"
#include "Utility/B3DRectOffset.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Contains items used for initializing one level in a drop down box hierarchy. */
	struct B3D_EXPORT GUIDropDownData
	{
		Vector<GUIDropDownDataEntry> Entries;
		Vector<bool> States;
		UnorderedMap<String, HString> LocalizedNames;
	};

	/**	A set of parameters used for initializing a drop down box. */
	struct DropDownBoxCreateInformation
	{
		HCamera Camera; /**< Camera on which to open the drop down box. */
		TDropDownAreaPlacement<GUIPhysicalUnit> Placement; /**< Determines how is the drop down box positioned in the visible area. */
		GUIDropDownData DropDownData; /**< Data to use for initializing menu items of the drop down box. */
		SPtr<const GUIStyleSheetCascade> StyleSheetCascade; /**< Style sheets to apply to drop down box GUI elements. */
		/** Additional bounds that control what is considered the inside or the outside of the drop down box. */
		Vector<GUIPhysicalArea> AdditionalBounds;
	};

	/**	Represents a single entry in a drop down box. */
	class B3D_EXPORT GUIDropDownDataEntry
	{
		enum class Type
		{
			Separator,
			Entry,
			SubMenu
		};

	public:
		/**	Creates a new separator entry. */
		static GUIDropDownDataEntry Separator();

		/** Creates a new button entry with the specified callback that is triggered when button is selected. */
		static GUIDropDownDataEntry Button(const String& label, std::function<void()> callback, const String& shortcutTag = StringUtility::kBlank);

		/** Creates a new sub-menu entry that will open the provided drop down data sub-menu when activated. */
		static GUIDropDownDataEntry SubMenu(const String& label, const GUIDropDownData& data);

		/**	Check is the entry a separator. */
		bool IsSeparator() const { return mType == Type::Separator; }

		/**	Check is the entry a sub menu. */
		bool IsSubMenu() const { return mType == Type::SubMenu; }

		/**	Returns display label of the entry (if an entry is a button or a sub-menu). */
		const String& GetLabel() const { return mLabel; }

		/**	Returns the shortcut key combination string that is to be displayed along the entry label. */
		const String& GetShortcutTag() const { return mShortcutTag; }

		/**	Returns a button callback if the entry (if an entry is a button). */
		std::function<void()> GetCallback() const { return mCallback; }

		/**	Returns sub-menu data that is used for creating a sub-menu (if an entry is a sub-menu). */
		const GUIDropDownData& GetSubMenuData() const { return mChildData; }

	private:
		GUIDropDownDataEntry() {}

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
	class B3D_EXPORT GUIDropDownMenu : public GUIWidget
	{
	public:
		/**
		 * Creates a new drop down box widget.
		 *
		 * @param	parent				Parent scene object to attach the drop down box to.
		 * @param	createInformation	Various parameters that control the drop down menu features and content.
		 * @param	type				Specific type of drop down box to display.
		 */
		GUIDropDownMenu(const HSceneObject& parent, const DropDownBoxCreateInformation& createInformation, GUIDropDownType type);

	private:
		/**	Contains data about a single drop down box sub-menu. */
		struct DropDownSubMenu
		{
			/**	Represents a single sub-menu page. */
			struct PageInfo
			{
				u32 Idx;
				u32 Start;
				u32 End;
				GUILogicalUnit Height;
			};

		public:
			/**
			 * Creates a new drop down box sub-menu.
			 *
			 * @param	owner			Owner drop down box this sub menu belongs to.
			 * @param	parent			Parent sub-menu. Can be null.
			 * @param	placement		Determines how is the sub-menu positioned in the visible area.
			 * @param	availableBounds	Available bounds (in pixels) in which the sub-menu may be opened.
			 * @param	dropDownData	Data to use for initializing menu items of the sub-menu.
			 * @param	type			Type of the drop down box to show.
			 * @param	depthOffset		How much to offset the sub-menu depth. We want deeper levels of the sub-menu
			 *							hierarchy to be in front of lower levels, so you should increase this value for
			 *							each level of the sub-menu hierarchy.
			 */
			DropDownSubMenu(GUIDropDownMenu* owner, DropDownSubMenu* parent, const TDropDownAreaPlacement<GUIPhysicalUnit>& placement, const GUIPhysicalArea& availableBounds, const GUIDropDownData& dropDownData, GUIDropDownType type, u32 depthOffset);
			~DropDownSubMenu();

			/**	Recreates all internal GUI elements for the entries of the current sub-menu page. */
			void UpdateGuiElements();

			/**	Moves the sub-menu to the previous page and displays its elements, if available. */
			void ScrollDown();

			/**	Moves the sub-menu to the next page and displays its elements, if available. */
			void ScrollUp();

			/**	Moves the sub-menu to the first page and displays its elements. */
			void ScrollToTop();

			/**	Moves the sub-menu to the last page and displays its elements. */
			void ScrollToBottom();

			/**	Calculates ranges for all the pages of the sub-menu. */
			Vector<PageInfo> GetPageInfos() const;

			/**
			 * Called when the user activates an element with the specified index.
			 *
			 * @param	index	Index of the activated element.
			 * @param	bounds	Bounds of the GUI element that is used as a visual representation of this drop down
			 *					element.
			 */
			void ElementActivated(u32 index, const GUIPhysicalArea& bounds);

			/**
			 * Called when the user selects an element with the specified index.
			 *
			 * @param	index	Index of the element that was selected.
			 */
			void ElementSelected(u32 index);

			/**	Called when the user wants to close the currently open sub-menu. */
			void CloseSubMenu();

			/**	Closes this sub-menu. */
			void Close();

			/**	Returns the type of the displayed drop down menu. */
			GUIDropDownType GetType() const { return Type; }

			/**	Returns actual visible bounds of the sub-menu. */
			GUIPhysicalArea GetVisibleBounds() const { return VisibleBounds; }

			/**	Returns the drop box object that owns this sub-menu. */
			GUIDropDownMenu* GetOwner() const { return Owner; }
		public:
			GUIDropDownMenu* Owner;

			GUIDropDownType Type;
			GUIDropDownData Data;
			u32 Page;
			GUILogicalPoint Position{kZeroTag};
			GUILogicalSize Size{kZeroTag};
			GUIPhysicalArea VisibleBounds;
			GUIPhysicalArea AvailableBounds;
			u32 DepthOffset;
			bool IsOpenedUpward;

			GUIDropDownContent* Content;
			GUITexture* BackgroundFrame;
			GUIButton* ScrollUpBtn;
			GUIButton* ScrollDownBtn;
			GUITexture* Handle;

			GUIPanel* BackgroundPanel;
			GUIPanel* ContentPanel;
			GUILayout* ContentLayout;
			GUIPanel* SidebarPanel;

			DropDownSubMenu* ParentSubMenu;
			DropDownSubMenu* ActiveChildSubMenu;
		};

	private:
		friend class GUIDropDownContent;

		/**	Called when the specified sub-menu is opened. */
		void NotifySubMenuOpened(DropDownSubMenu* subMenu);

		/**	Called when the specified sub-menu is opened. */
		void NotifySubMenuClosed(DropDownSubMenu* subMenu);

		/**	Called when the drop down box loses focus (and should be closed). */
		void DropDownFocusLost();

		void OnCreated() override;
		void OnDestroyed() override;

	private:
		static const GUILogicalUnit kDropDownBoxWidth;
		static constexpr const char* kBackgroundFrameStyleClass = "GUIDropDownBackgroundFrame";
		static constexpr const char* kScrollbarBackgroundStyleClass = "GUIDropDownScrollbarBackground";
		static constexpr const char* kScrollbarButtonStyleClass = "GUIDropDownScrollbarButton";
		static constexpr const char* kScrollbarHandleStyleClass = "GUIDropDownScrollbarHandle";

		DropDownBoxCreateInformation mMenuCreateInformation;
		GUIDropDownType mType;
		RectOffset mBackgroundFramePadding;
		GUILogicalUnit mScrollbarWidth = 0;
		GUILogicalUnit mScrollButtonHeight = 0;
		DropDownSubMenu* mRootMenu = nullptr;
		GUIDropDownHitBox* mFrontHitBox = nullptr;
		GUIDropDownHitBox* mBackHitBox = nullptr;

		// Captures mouse clicks so that we don't trigger elements outside the drop down box when we just want to close it.
		// (Particular example is clicking on the button that opened the drop down box in the first place. Clicking will cause
		// the drop down to lose focus and close, but if the button still processes the mouse click it will be immediately opened again)
		GUIDropDownHitBox* mCaptureHitBox = nullptr;

		Vector<GUIPhysicalArea> mAdditionalCaptureBounds;
	};

	/** @} */
} // namespace b3d
