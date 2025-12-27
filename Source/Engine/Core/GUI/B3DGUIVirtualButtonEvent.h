//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DVirtualInput.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	Holds data about a GUI event that happens when a virtual button is pressed. */
	class B3D_EXPORT GUIVirtualButtonEvent
	{
	public:
		GUIVirtualButtonEvent() = default;

		/**	Returns the virtual button the event is referring to. */
		const VirtualButton& GetButton() const { return mButton; }

	private:
		friend class GUIManager;

		/**	Initializes the data for the event. */
		void SetButton(const VirtualButton& button) { mButton = button; }

		VirtualButton mButton;
	};

	/** @} */
} // namespace b3d
