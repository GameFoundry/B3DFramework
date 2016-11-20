//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsModule.h"

namespace bs
{
	/** @addtogroup EditorWindow-Internal
	 *  @{
	 */

	/**
	 * Handles opening and closing of EditorWidgets. Its primary purpose is to keep track of all types of widgets so they
	 * can be saved and restored upon program shutdown/startup, as well as being able to change widget layout on the fly.
	 */
	class BS_ED_EXPORT EditorWidgetManager : public Module<EditorWidgetManager>
	{
	public:
		EditorWidgetManager();
		~EditorWidgetManager();

		/**	Called every frame. */
		void update();

		/**
		 * Registers a widget that can then be opened by calling open(). When loading a widget layout this name and callback
		 * will be used to attempt creating the widget.
		 *
		 * @param[in]	name				  	Unique name for the widget.
		 * @param[in]	createCallback			Callback that returns a new instance of the widget.
		 */
		void registerWidget(const String& name, std::function<EditorWidgetBase*(EditorWidgetContainer&)> createCallback);

		/**	Unregisters a widget so it may no longer be opened using this manager. */
		void unregisterWidget(const String& name);

		/**
		 * Creates a widget with the given name. If widget is already created it returns the existing instance.	Widget is
		 * opened in a new window.
		 *
		 * @param[in]	name	The name of the widget.
		 * @return				Always returns the created widget, and throws an exception if it fails.
		 */
		EditorWidgetBase* open(const String& name);

		/**	Creates a new widget an inserts it into the specified container. */
		EditorWidgetBase* create(const String& name, EditorWidgetContainer& parentContainer);

		/**	Checks if the provided name represents a widget that can be created. */
		bool isValidWidget(const String& name) const;

		/**	Closes the given widget. */
		void close(EditorWidgetBase* widget);

		/**	Closes all open editor widgets. */
		void closeAll();

		/**
		 * Retrieves the layout of all currently active widgets. You may later use this layout to restore exact position of
		 * the widgets.
		 */
		SPtr<EditorWidgetLayout> getLayout() const;

		/**
		 * Positions all widgets according to the provided layout. It will open new widgets or close current ones if needed.
		 */
		void setLayout(const SPtr<EditorWidgetLayout>& layout);

		/**
		 * Allows you to queue up widgets that will be registered as soon as an instance of EditorWidgetManager is created.
		 *
		 * @note	
		 * Useful primarily when widgets are being registered from static methods, because then there is no
		 * EditorWidgetManager instance yet.
		 */
		static void preRegisterWidget(const String& name, std::function<EditorWidgetBase*(EditorWidgetContainer&)> createCallback);

	private:
		/**	Triggered whenever a window gains focus. */
		void onFocusGained(const RenderWindow& window);

		/**	Triggered whenever a window loses focus. */
		void onFocusLost(const RenderWindow& window);

		Map<String, EditorWidgetBase*> mActiveWidgets;
		Map<String, std::function<EditorWidgetBase*(EditorWidgetContainer&)>> mCreateCallbacks;

		HEvent mOnFocusLostConn;
		HEvent mOnFocusGainedConn;

		static Stack<std::pair<String, std::function<EditorWidgetBase*(EditorWidgetContainer&)>>> QueuedCreateCallbacks;
	};

	/** @} */
}