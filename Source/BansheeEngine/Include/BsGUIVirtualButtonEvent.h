//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsVirtualInput.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Holds data about a GUI event that happens when a virtual button is pressed. */
	class BS_EXPORT GUIVirtualButtonEvent
	{
	public:
		GUIVirtualButtonEvent()
		{ }

		/**	Returns the virtual button the event is referring to. */
		const VirtualButton& getButton() const { return mButton; }
	private:
		friend class GUIManager;
		
		/**	Initializes the data for the event. */
		void setButton(const VirtualButton& button) { mButton = button; }

		VirtualButton mButton;
	};

	/** @} */
}