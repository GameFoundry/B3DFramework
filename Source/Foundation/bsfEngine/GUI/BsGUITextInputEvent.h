//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"
#include "Input/BsInputFwd.h"
#include "Math/BsVector2I.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	A text input event representing input of a single character. */
	class BS_EXPORT GUITextInputEvent
	{
	public:
		GUITextInputEvent() = default;

		/**	Character code that was input. */
		const UINT32& getInputChar() const { return mInputChar; }
	private:
		friend class GUIManager;

		/**	Initializes the event data with the character that was input. */
		void setData(UINT32 inputChar);

		UINT32 mInputChar = 0;
	};

	/** @} */
}