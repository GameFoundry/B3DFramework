//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEvent.h"

namespace BansheeEngine
{
	/** @addtogroup EditorWindow-Internal
	 *  @{
	 */

	/**
	 * A GUI object that contains one or multiple editor widgets. Each widget is represented by a single tab and can be
	 * activated, deactivated, moved or dragged off.
	 */
	class BS_ED_EXPORT EditorWidgetContainer
	{
	public:
		EditorWidgetContainer(GUIWidget* parent, EditorWindowBase* parentEditorWindow);
		virtual ~EditorWidgetContainer();

		/**	Adds a new widget to the container, adding a new tab to the end of the tab bar. */
		void add(EditorWidgetBase& widget);

		/**	Removes a widget from the container. */
		void remove(EditorWidgetBase& widget);

		/**
		 * Inserts a widget at the specified index, adding a new tab at that position.
		 *
		 * @param[in]	idx		Sequential index where to insert the widget.
		 * @param[in]	widget	Widget to insert.
		 */
		void insert(UINT32 idx, EditorWidgetBase& widget);

		/**	Checks if the container already contains the provided widget. */
		bool contains(EditorWidgetBase& widget);

		/**	Sets the size of the container in pixels. This also modifies the size of all child widgets. */
		void setSize(UINT32 width, UINT32 height);

		/**
		 * Sets the position of the container, relative to the parent GUI widget. This also modifies the position of all
		 * child widgets.
		 */
		void setPosition(INT32 x, INT32 y);

		/**
		* Changes the currently active widget to the one at the specified index. Making the widget active means it will be
		* visible in the container.
		*
		* @param[in]	idx		Unique widget index (not sequential).
		*/
		void setActiveWidget(UINT32 idx);

		/**	Returns the number of widgets currently docked in this container. */
		UINT32 getNumWidgets() const { return (UINT32)mWidgets.size(); }

		/**
		 * Returns a widget at the specified index.
		 *
		 * @param[in]	idx		Sequential index of the widget to retrieve.
		 */
		EditorWidgetBase* getWidget(UINT32 idx) const;

		/**	Returns a widget that is currently visible (its tab is active). */
		EditorWidgetBase* getActiveWidget() const;

		/**	Returns the parent GUI widget the container is using to render the GUI on. */
		GUIWidget& getParentWidget() const { return *mParent; }

		/**	Returns the parent editor window the container is docked in. */
		EditorWindowBase* getParentWindow() const { return mParentWindow; }

		/** Returns bounds not including the tabbed title bar. These are the bounds available to child widget GUI. */
		Rect2I getContentBounds() const;

		/**	Returns a list of areas that can be dragged off. These are normally areas represented by tab buttons. */
		Vector<Rect2I> getDraggableAreas() const;

		/**	Called once per frame. Calls update on all docked widgets. */
		void update();

		/**	Updates the tabbed title bar by refreshing the names of all docked widgets. */
		void refreshWidgetNames();

		/**	Triggers when a widget is about to be destroyed. */
		void _notifyWidgetDestroyed(EditorWidgetBase* widget);

		/**
		 * Returns the size of a widget docked in a window of the specified size. Window and widget sizes are different due
		 * to the title bar and potentially other window-specific GUI elements.
		 */
		static Vector2I windowToWidgetSize(const Vector2I& windowSize);

		/**
		 * Returns the size of a window required for displaying a widget of the specified size. Window and widget sizes are
		 * different due to the title bar and potentially other window-specific GUI elements.
		 */
		static Vector2I widgetToWindowSize(const Vector2I& widgetSize);

		Event<void()> onWidgetAdded; /**< Triggered whenever a new widget is added to this container. */
		Event<void()> onWidgetClosed; /**< Triggered whenever a widget docked in this container is closed. */
		Event<void()> onMaximized; /**< Triggered when the maximize button is clicked. */

		static const UINT32 TitleBarHeight;
	private:
		/**	Removes a widget without triggering a widget closed event. */
		void removeInternal(EditorWidgetBase& widget);

		/**
		 * Triggered when a user clicks on a tab in the tabbed title bar.
		 *
		 * @param[in]	idx		Unique widget index (not sequential) of the tab that was activated.
		 */
		void tabActivated(UINT32 idx);

		/**
		 * Triggered when a user closes a tab in the tabbed title bar.
		 *
		 * @param[in]	idx		Unique widget index (not sequential) of the tab that was closed.
		 */
		void tabClosed(UINT32 idx);

		/**
		 * Triggered when a user clicks the maximize button on the title bar.
		 *
		 * @param[in]	idx		Unique widget index (not sequential) of the tab that was closed.
		 */
		void tabMaximized(UINT32 idx);

		/**
		 * Triggered when a user drags a tab off the tabbed title bar.
		 *
		 * @param[in]	idx		Unique widget index (not sequential) of the tab that was dragged off.
		 */
		void tabDraggedOff(UINT32 idx);

		/**
		 * Triggered when a user drags a tab on the tabbed title bar.
		 *
		 * @param[in]	idx		Sequential index at the position on which the tab has been dragged on.
		 */
		void tabDraggedOn(UINT32 idx);

		/**
		 * Triggered when the widget drag and drop operation finishes.
		 *
		 * @param[in]	wasDragProcessed	Signals whether any object handled the drop.
		 */
		static void tabDroppedCallback(bool wasDragProcessed);

		EditorWindowBase* mParentWindow;
		GUITabbedTitleBar* mTitleBar;
		GUIPanel* mTitleBarPanel;
		GUIWidget* mParent;
		INT32 mX, mY;
		UINT32 mWidth, mHeight;
		UnorderedMap<UINT32, EditorWidgetBase*> mWidgets;
		INT32 mActiveWidget;
	};

	/** @} */
}