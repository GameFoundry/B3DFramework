//************************************ B3D Framework - Copyright 2026 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "B3DUtilityPrerequisites.h"
#include "Utility/B3DModule.h"

namespace b3d
{
	/** @addtogroup System
	 *  @{
	 */

	/**
	 * This manager keeps track of all the open dynamic-loading libraries, it manages opening them opens them and can be
	 * used to lookup already already-open libraries.
	 *
	 * @note	Not thread safe.
	 */
	class B3D_EXPORT DynamicLibraryManager : public Module<DynamicLibraryManager>
	{
	public:
		/**
		 * Loads the given file as a dynamic library.
		 *
		 * @param	name	The name of the library. The extension can be omitted.
		 */
		DynamicLibrary* Load(String name);

		/** Unloads the given library. */
		void Unload(DynamicLibrary* lib);

	protected:
		Set<UPtr<DynamicLibrary>, std::less<>> mLoadedLibraries;
	};

	/** Easy way of accessing DynLibManager. */
	B3D_EXPORT DynamicLibraryManager& GetDynamicLibraryManager();

	/** @} */
} // namespace b3d
