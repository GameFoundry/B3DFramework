//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsPrerequisites.h"
#include "BsInputFwd.h"
#include "BsVector2I.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	A text input event representing input of a single character. */
	class BS_EXPORT GUITextInputEvent
	{
	public:
		GUITextInputEvent();

		/**	Character code that was input. */
		const UINT32& getInputChar() const { return mInputChar; }
	private:
		friend class GUIManager;

		/**	Initializes the event data with the character that was input. */
		void setData(UINT32 inputChar);

		UINT32 mInputChar;
	};

	/** @} */
}