//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsEditorPrerequisites.h"
#include "BsEditorWindowBase.h"

namespace bs
{
	/** @addtogroup EditorWindow
	 *  @{
	 */

	/**	A draggable and resizeable window that has a single widget container you may use for docking editor widgets in. */
	class BS_ED_EXPORT EditorWindow : public EditorWindowBase
	{
	public:
		virtual ~EditorWindow();

		/**	Returns the widget container for this window. */
		EditorWidgetContainer& widgets() const { return *mWidgets; }

		/** @copydoc EditorWindowBase::update  */
		virtual void update() override;

		/**	Creates a new empty editor window. */
		static EditorWindow* create();

	protected:
		friend class EditorWindowManager;
		EditorWindow();

		/** @copydoc EditorWindowBase::resized  */
		virtual void resized() override;
	private:
		EditorWidgetContainer* mWidgets;

		/**
		 * Updates the sizes of the child widget container and OS non-client areas (like drag and resize). Call when window
		 * size changes.
		 */
		void updateSize();

		/**	Called when a widget is added to the widget container. */
		void widgetAdded();

		/**	Called when a widget is removed from the widget container. */
		void widgetRemoved();

		/**	Triggered when the maximize button on the title bar is clicked. */
		void maximizeClicked();

		/**
		 * A callback that triggers when a drag and drop operation originated from this window ends. 
		 *
		 * @note	
		 * This is a workaround to get around the problem that closing the window when the drag operation starts (for 
		 * example if the last widget is dragged from the window) will cause the application to lose mouse capture and will
		 * not receive mouse events. Therefore we delay the window closing until the drag ends.
		 */
		void closeWindowDelayed();
	};

	/** @} */
}