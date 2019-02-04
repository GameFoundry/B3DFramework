//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Input/BsVirtualInput.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Holds data about a GUI event that happens when a virtual button is pressed. */
	class BS_EXPORT GUIVirtualButtonEvent
	{
	public:
		GUIVirtualButtonEvent() = default;

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