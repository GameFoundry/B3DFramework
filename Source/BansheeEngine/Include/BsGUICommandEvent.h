//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Type of valid command events. */
	enum class GUICommandEventType
	{
		Redraw, /**< GUI system is forcing the GUI element to redraw itself. */
		FocusLost, /**< GUI element lost input focus. */
		FocusGained, /**< GUI element gained input focus. */
		MoveLeft, /**< Input caret was moved left (for example for navigating an input box). */
		MoveRight, /**< Input caret was moved right (for example for navigating an input box). */
		MoveUp, /**< Input caret was moved up (for example for navigating an input box). */ 
		MoveDown, /**< Input caret was moved down (for example for navigating an input box). */ 
		SelectLeft, /**< Input Selection was moved left (for example for selecting text in an input box). */ 
		SelectRight, /**< Input Selection was moved right (for example for selecting text in an input box). */ 
		SelectUp, /**< Input Selection was moved up (for example for selecting text in an input box). */ 
		SelectDown, /**< Input Selection was moved down (for example for selecting text in an input box). */ 
		Escape, /**< Escape key was pressed. */
		Delete, /**< Delete key was pressed. */
		Backspace, /**< Backspace key was pressed. */
		Return, /**< Shift + Enter was pressed. */
		Confirm /**< Enter key was pressed. */
	};

	/**
	 * Holds data about a GUI command event. Command events are special events with a more specific purpose than general
	 * input events.
	 */
	class BS_EXPORT GUICommandEvent
	{
	public:
		GUICommandEvent()
			:mType(GUICommandEventType::Redraw)
		{ }

		/**	Returns type describing what kind of event this is. */
		GUICommandEventType getType() const { return mType; }
	private:
		friend class GUIManager;

		/**	Sets type describing what kind of event this is. */
		void setType(GUICommandEventType type) { mType = type; }

		GUICommandEventType mType;
	};

	/** @} */
}