//************************************ B3D Framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"
#include "GUI/B3DGUIDropDownMenu.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup GUI-Internal
	 *  @{
	 */

	/**	Manages opening and closing of a drop down box. */
	class B3D_EXPORT GUIDropDownBoxManager : public Module<GUIDropDownBoxManager>
	{
	public:
		~GUIDropDownBoxManager();

		/**
		 * Opens a new drop down box at the specified location, look and elements. This will close any previously open drop
		 * down box.
		 *
		 * @param	createInformation	Various parameters for initializing the drop down box.
		 * @param	type				Specific type of drop down box to display.
		 * @param	onClosedCallback	Callback triggered when drop down box is closed.
		 */
		TGameObjectHandle<GUIDropDownMenu> OpenDropDownBox(const DropDownBoxCreateInformation& createInformation, GUIDropDownType type, std::function<void()> onClosedCallback);

		/**	Closes the currently active drop down box (if any). */
		void CloseDropDownBox();

	private:
		HSceneObject mDropDownSO;
		TGameObjectHandle<GUIDropDownMenu> mDropDownBox;
		std::function<void()> mOnClosedCallback;
	};

	/** @} */
} // namespace b3d
