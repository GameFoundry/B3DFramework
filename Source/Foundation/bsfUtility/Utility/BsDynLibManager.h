//************************************ bs::framework - Copyright 2018 Marko Pintera **************************************//
//*********** Licensed under the MIT license. See LICENSE.md for full terms. This notice is not to be removed. ***********//
#pragma once

#include "Prerequisites/BsPrerequisitesUtil.h"
#include "Utility/BsModule.h"

namespace bs
{
	/** @addtogroup General
	 *  @{
	 */

	/**
	 * This manager keeps a track of all the open dynamic-loading libraries, opens them and returns references to
	 * already-open libraries.
	 *
	 * @note	Not thread safe.
	 */
	class BS_UTILITY_EXPORT DynLibManager : public Module<DynLibManager>
	{
	public:
		DynLibManager() = default;
		virtual ~DynLibManager();

		/**
		 * Loads the given file as a dynamic library.
		 *
		 * @param[in]	name	The name of the library. The extension can be omitted.
		 */
		DynLib* load(String name);

		/** Unloads the given library. */
		void unload(DynLib* lib);

	protected:
		Set<NPtr<DynLib>, std::less<>> mLoadedLibraries;
	};

	/** Easy way of accessing DynLibManager. */
	BS_UTILITY_EXPORT DynLibManager& gDynLibManager();

	/** @} */
}
