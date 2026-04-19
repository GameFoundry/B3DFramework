//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DPrerequisites.h"

namespace b3d
{
	class GUIToggleable;

	/** @addtogroup GUI
	 *  @{
	 */

	/** Object that allows you to group multiple GUI toggle buttons. Only one button among the grouped ones can be active. */
	class B3D_EXPORT B3D_SCRIPT_EXPORT(DocumentationGroup(GUI)) GUIToggleGroup : public IScriptExportable
	{
	public:
		~GUIToggleGroup();

		/**
		 * Creates a toggle group that you may provide to GUIToggleable upon construction. Toggles sharing the same group will
		 * only have a single element active at a time.
		 *
		 * @param	allowAllOff	If true all of the toggle buttons can be turned off, if false one will always be turned on.
		 */
		B3D_SCRIPT_EXPORT(ExtensionConstructorForType(GUIToggleGroup))
		static SPtr<GUIToggleGroup> Create(bool allowAllOff = false);

	public: // ***** INTERNAL ******
		/** @name Internal
		 *  @{
		 */

		/** Registers a new toggle button with the group. */
		void AddInternal(GUIToggleable* toggle);

		/**	Unregisters a toggle button from the group. */
		void RemoveInternal(GUIToggleable* toggle);

		/** @} */
	private:
		friend class GUIToggleable;

		GUIToggleGroup(bool allowAllOff);

		/**	Initializes the toggle group. To be called right after construction. */
		void Initialize(const SPtr<GUIToggleGroup>& sharedPtr);

		Vector<GUIToggleable*> mButtons;
		bool mAllowAllOff;
		std::weak_ptr<GUIToggleGroup> mThis;
	};

	/** @} */
} // namespace b3d
