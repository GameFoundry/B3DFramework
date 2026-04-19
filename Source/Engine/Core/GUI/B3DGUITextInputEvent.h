//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "Input/B3DInputFwd.h"

namespace b3d
{
	/** @addtogroup GUI
	 *  @{
	 */

	/**	A text input event representing input of a single character. */
	class B3D_EXPORT GUITextInputEvent
	{
	public:
		GUITextInputEvent() = default;

		/**	Character code that was input. */
		const u32& GetInputChar() const { return mInputChar; }

	private:
		friend class GUIManager;

		/**	Initializes the event data with the character that was input. */
		void SetData(u32 inputChar);

		u32 mInputChar = 0;
	};

	/** @} */
} // namespace b3d
