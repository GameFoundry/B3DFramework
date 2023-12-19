//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "BsPrerequisites.h"

namespace bs
{
	/** @addtogroup GUI
	 *  @{
	 */

	/** Object that allows you to group multiple GUI toggle buttons. Only one button among the grouped ones can be active. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIToggleGroup
	{
	public:
		~GUIToggleGroup();

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Registers a new toggle button with the group. */
		void AddInternal(GUIToggle* toggle);

		/**	Unregisters a toggle button from the group. */
		void RemoveInternal(GUIToggle* toggle);

		/** @} */
	private:
		friend class GUIToggle;

		GUIToggleGroup(bool allowAllOff);

		/**	Initializes the toggle group. To be called right after construction. */
		void Initialize(const SPtr<GUIToggleGroup>& sharedPtr);

		Vector<GUIToggle*> mButtons;
		bool mAllowAllOff;
		std::weak_ptr<GUIToggleGroup> mThis;
	};

	/** @} */
} // namespace bs
